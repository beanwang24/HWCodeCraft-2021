#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <set>
#include "iostream"
#include <string>
#include <algorithm>

using namespace std;

int Day = 0, OpCntofOneDay = 0;
int K = 0;
int CpuNeedAll = 0, MemNeedAll = 0;
int CpuHave = 0, MemHave = 0;

//控制放入后两种资源剩余的差距不能过大
int p1 = 5;
int p2 = 35;

//主机种类
struct HostCategory
{
    string name;
    int CoreNum;
    int Memory;
    int Prices;
    int EleCharge;
    double myPrice;

    bool operator < (const HostCategory &a)
    {
        return this->myPrice < a.myPrice;
    }
};

//布置了的虚拟机
struct Vm
{
    string VmName;
    int HostId;  //存放在哪个主机上
    int AB; //0是双节点 1放在A 2 放在B节点
};

//买了的主机
struct Host
{
    string HostName;
    int HostId;
    int CoreCapacity;    // 原本有的cpu
    int MemoryCapacity;  // 原本有的cpu
    int CoreNum_A;
    int CoreNum_B;
    int Memory_A;
    int Memory_B;
    int EleCharge;
    unordered_set<int> VmOnThisHost; //存放改主机上 部署了的 虚拟机的id


    bool operator < (const Host &a)
    {
//        double this_notUseRatio = (double) (CoreNum_A + CoreNum_B) * (Memory_A + Memory_B) / CoreCapacity / MemoryCapacity;
//        double a_notUseRatio = ((double) (a.CoreNum_A + a.CoreNum_B) * (a.Memory_A + a.Memory_B) / a.CoreCapacity / a.MemoryCapacity);

        double this_notUseRatio = max((double) (CoreNum_A + CoreNum_B) / CoreCapacity, (double)(Memory_A + Memory_B) / MemoryCapacity);
        double a_notUseRatio    = max((double) (a.CoreNum_A + a.CoreNum_B) / a.CoreCapacity, (double)(a.Memory_A + a.Memory_B) / a.MemoryCapacity);
//        return this_notUseRatio < a_notUseRatio;

//        double this_notUseRatio = 1.0 * (CoreNum_A + CoreNum_B + Memory_A + Memory_B) / (CoreCapacity + MemoryCapacity);
//        double a_notUseRatio    = 1.0 * (a.CoreNum_A + a.CoreNum_B + a.Memory_A + a.Memory_B) / (a.CoreCapacity + a.MemoryCapacity);
        return this_notUseRatio < a_notUseRatio;
    }
};

//一次add请求
struct Request_Add
{
    //这是该天的第几次请求
    int Order;
    //这是该天的第几次Add请求
    int Order_Add;

    string VmName;
    int VmId;
    int CpuNeed;
    int Memory_Need;

    //0位单节点
    int Mode;

    bool operator < (const Request_Add &a)
    {
        //由大到小
        int cnt1 = CpuNeed + Memory_Need;
        int cnt2 = a.CpuNeed + a.Memory_Need;
        if(Mode == 0) cnt1 *= 2;
        if(a.Mode == 0) cnt2 *= 2;
        return cnt1 > cnt2;

    }
};

//一次Del请求
struct Request_Del
{
    //这是该天的第几次请求
    int Order;
    //这是该天的第几次Del请求
    int Order_Del;
    int VmId;
    int HostId;
    //将会在分别哪个结点上，减去什么
    int Cpu_A;
    int Cpu_B;
    int Mem_A;
    int Mem_B;

    //0为还没有执行过， 1为已经执行过该次Del操作
    int flag = 0;

    bool operator < (const Request_Del &a)
    {
        //由大到小，先去使用顺序靠后的
        return Order > a.Order;
    }
};

//存放主机种类的数组
vector<HostCategory> HostCate;

//存储虚拟机种类
unordered_map<string, vector<int>> VmCategory;

//存储每天的操作,pair<操作次数，具体操作的数组>
vector<pair<int, vector<vector<string>>>> DayOp;

//存储买入的主机
vector<Host> HostBuy;

//存储布置了的虚拟机 <int vmid, Vm>
unordered_map<int, Vm> VmDeploy;

//映射id 用于输出
unordered_map <int, int> HostIdMapping;

unordered_map<int, string> VmIdtoVmName;

/*************某天操作的变量的存储************/
int VmNum = 0;

int DaysNum = 0;

//主机id计数
int HostIdCnt = 0;

int HostIdCntForMapping = 0;

//该天买入的类型的数量
int PurchaseTypeNum;

//该天迁移虚拟机的数量
int Migration_Num;

//该天买入的服务器的名称和数量
unordered_map<string, int> OneDayBuyHost;

//部署的操作 第一个int为hostid,第二个0位双节点，1位A, 2位B
vector<pair<int, int>> DeployOp;

//迁移的操作 分别是虚拟机ID，主机id，节点吗，模式（0为双，1A,2B)
vector<vector<int>> MigrationOp;

unordered_set<int> AlreadyMigVm; //存放这次迁徙 已经被迁徙了 的VmId;

//保存该天的请求
vector<Request_Add> OneDayAdd_VC;
unordered_map<int, vector<Request_Del>> OneDayDel_VC;

/******************************************/


/***********************功能函数*******************************/
//存入主机种类
void SaveHost(string name, string CoreNum, string Memory, string Prices, string EleCharge)
{
    HostCategory tmp;
    name = name.substr(1, name.size() - 2);

    int tmpCoreNum = 0, tmpMemory = 0, tmpPrices = 0, tmpEleCharge = 0;
    for(int i = 0; i < CoreNum.size() - 1; ++i)       tmpCoreNum = tmpCoreNum * 10 + CoreNum[i] - '0';
    for(int i = 0; i < Memory.size() - 1; ++i)        tmpMemory = tmpMemory * 10 + Memory[i] - '0';
    for(int i = 0; i < Prices.size() - 1; ++i)        tmpPrices = tmpPrices * 10 + Prices[i] - '0';
    for(int i = 0; i < EleCharge.size() - 1; ++i)     tmpEleCharge = tmpEleCharge * 10 + EleCharge[i] - '0';

    tmp.name = name;
    tmp.CoreNum = tmpCoreNum;
    tmp.Memory = tmpMemory;
    tmp.Prices = tmpPrices;
    tmp.EleCharge = tmpEleCharge;
    HostCate.push_back(tmp);
}

//存入虚拟机种类 0为单节点
void SaveVm(string name, string CoreNum, string Memory, string Modes)
{

    HostCategory tmp;
    name = name.substr(1, name.size() - 2);

    int tmpCoreNum = 0, tmpMemory = 0, tmpModes = 0;
    for(int i = 0; i < CoreNum.size() - 1; ++i)       tmpCoreNum = tmpCoreNum * 10 + CoreNum[i] - '0';
    for(int i = 0; i < Memory.size() - 1; ++i)        tmpMemory = tmpMemory * 10 + Memory[i] - '0';

    if(Modes[0] == '1')                               tmpModes = 1;

    VmCategory[name] = vector<int> {tmpCoreNum, tmpMemory, tmpModes};
}

//存储add请求
void SaveOp(int Day, string op, string VmName, string VmId)
{
    op = op.substr(1, op.size() - 2);
    VmName = VmName.substr(0, VmName.size() - 1);
    VmId = VmId.substr(0, VmId.size() - 1);
    DayOp[Day].second.push_back(vector<string>{op, VmName, VmId});

    CpuNeedAll += VmCategory[VmName][0];
    MemNeedAll += VmCategory[VmName][1];

    VmIdtoVmName[stoi(VmId)] = VmName;
}

//存储del请求
void SaveOp(int Day, string op, string VmId)
{
    op = op.substr(1, op.size() - 2);
    VmId = VmId.substr(0, VmId.size() - 1);
    DayOp[Day].second.push_back(vector<string>{op, VmId});


    string VmName = VmIdtoVmName[stoi(VmId)];
    CpuNeedAll -= VmCategory[VmName][0];
    MemNeedAll -= VmCategory[VmName][1];
}

//删除虚拟机
void DelVm(int VmId)
{
    --VmNum;
    string VmName = VmDeploy[VmId].VmName;
    int HostId = VmDeploy[VmId].HostId;
    int AB = VmDeploy[VmId].AB;

    int cpu = VmCategory[VmName][0];
    int memory = VmCategory[VmName][1];
    int Modes = VmCategory[VmName][2];

    int index = 0;
    for(int i = 0; i < HostBuy.size(); ++i)
    {
        if(HostBuy[i].HostId == HostId)
        {
            index = i;
            break;
        }
    }

    if(Modes == 1)
    {
        HostBuy[index].CoreNum_A += cpu / 2;
        HostBuy[index].CoreNum_B += cpu / 2;
        HostBuy[index].Memory_A  += memory / 2;
        HostBuy[index].Memory_B  += memory / 2;
    }
    else
    {
        if(AB == 1)
        {
            HostBuy[index].CoreNum_A += cpu;
            HostBuy[index].Memory_A  += memory;
        }
        else
        {
            HostBuy[index].CoreNum_B += cpu;
            HostBuy[index].Memory_B  += memory;
        }
    }

    //删除
    HostBuy[index].VmOnThisHost.erase(VmId);
    VmDeploy.erase(VmId);
}

//删除虚拟机
void DelVm_v2(int VmId, int Cpu_A, int Mem_A, int Cpu_B, int Mem_B)
{
    --VmNum;
    string VmName = VmDeploy[VmId].VmName;
    int HostId = VmDeploy[VmId].HostId;
    int AB = VmDeploy[VmId].AB;

    int index = 0;
    for(int i = 0; i < HostBuy.size(); ++i)
    {
        if(HostBuy[i].HostId == HostId)
        {
            index = i;
            break;
        }
    }

    //归还资源
    HostBuy[index].CoreNum_A += Cpu_A;
    HostBuy[index].CoreNum_B += Cpu_B;
    HostBuy[index].Memory_A  += Mem_A;
    HostBuy[index].Memory_B  += Mem_B;

    //删除
    HostBuy[index].VmOnThisHost.erase(VmId);
    VmDeploy.erase(VmId);
}

//添加双节点虚拟机
void AddVm(string VmName, int VmId, int HostId)
{
    ++VmNum;
    int cpu = VmCategory[VmName][0];
    int memory = VmCategory[VmName][1];

    Vm tmpVm;
    tmpVm.VmName = VmName;
    tmpVm.HostId = HostId;
    tmpVm.AB = 0;

    //部署
    VmDeploy[VmId] = tmpVm;

    //对主机操作
    int index = 0;
    for(int i = 0; i < HostBuy.size(); ++i)
    {
        if(HostBuy[i].HostId == HostId)
        {
            index = i;
            break;
        }
    }

    HostBuy[index].CoreNum_A -= cpu / 2;
    HostBuy[index].CoreNum_B -= cpu / 2;
    HostBuy[index].Memory_A  -= memory /2;
    HostBuy[index].Memory_B  -= memory /2;

    HostBuy[index].VmOnThisHost.insert(VmId);

}

//添加单节点虚拟机 ABtype 1放在A上面 2 放在B上面
void AddVm(string VmName, int VmId, int HostId, int ABtype)
{
    ++VmNum;
    int cpu = VmCategory[VmName][0];
    int memory = VmCategory[VmName][1];

    Vm tmpVm;
    tmpVm.VmName = VmName;
    tmpVm.HostId = HostId;
    tmpVm.AB = ABtype;

    //部署
    VmDeploy[VmId] = tmpVm;

    //对主机操作
    int index = 0;
    for(int i = 0; i < HostBuy.size(); ++i)
    {
        if(HostBuy[i].HostId == HostId)
        {
            index = i;
            break;
        }
    }

    if(ABtype == 1)
    {
        HostBuy[index].CoreNum_A -= cpu;
        HostBuy[index].Memory_A  -= memory;
    }
    else if(ABtype == 2)
    {
        HostBuy[index].CoreNum_B -= cpu;
        HostBuy[index].Memory_B -= memory;
    }
    HostBuy[index].VmOnThisHost.insert(VmId);
}

void AddVm_v2(string VmName, int VmId, int HostId, int ABtype, int CpuNeed, int MemNeed)
{
    ++VmNum;

    Vm tmpVm;
    tmpVm.VmName = VmName;
    tmpVm.HostId = HostId;
    tmpVm.AB = ABtype;

    //部署
    VmDeploy[VmId] = tmpVm;

    //对主机操作
    int index = 0;
    for(int i = 0; i < HostBuy.size(); ++i)
    {
        if(HostBuy[i].HostId == HostId)
        {
            index = i;
            break;
        }
    }

    if(ABtype == 1)
    {
        HostBuy[index].CoreNum_A -= CpuNeed;
        HostBuy[index].Memory_A  -= MemNeed;
    }
    else if(ABtype == 2)
    {
        HostBuy[index].CoreNum_B -= CpuNeed;
        HostBuy[index].Memory_B -= MemNeed;
    }
    HostBuy[index].VmOnThisHost.insert(VmId);
}

void AddVm_v2(string VmName, int VmId, int HostId, int CpuNeed_A, int MemNeed_A, int CpuNeed_B, int MemNeed_B)
{
    ++VmNum;

    Vm tmpVm;
    tmpVm.VmName = VmName;
    tmpVm.HostId = HostId;
    tmpVm.AB = 0;

    //部署
    VmDeploy[VmId] = tmpVm;

    //对主机操作
    int index = 0;
    for(int i = 0; i < HostBuy.size(); ++i)
    {
        if(HostBuy[i].HostId == HostId)
        {
            index = i;
            break;
        }
    }

    HostBuy[index].CoreNum_A -= CpuNeed_A;
    HostBuy[index].Memory_A  -= MemNeed_A;


    HostBuy[index].CoreNum_B -= CpuNeed_B;
    HostBuy[index].Memory_B -= MemNeed_B;

    HostBuy[index].VmOnThisHost.insert(VmId);
}

//买入一个主机 并且进行一些初始化
void BuyHost(string HostName, int HostId, int CoreCapacity, int MemoryCapacity, int CoreNum_A, int CoreNum_B, int Memory_A, int Memory_B, int EleCharge)
{
    Host HostTmp;
    HostTmp.HostName = HostName;
    HostTmp.HostId = HostId;
    HostTmp.CoreCapacity = CoreCapacity;
    HostTmp.MemoryCapacity = MemoryCapacity;
    HostTmp.CoreNum_A = CoreNum_A;
    HostTmp.CoreNum_B = CoreNum_B;
    HostTmp.Memory_A = Memory_A;
    HostTmp.Memory_B = Memory_B;
    HostTmp.EleCharge = EleCharge;

    HostBuy.push_back(HostTmp);

    CpuHave += CoreCapacity;
    MemHave += MemoryCapacity;
}

//添加操作
void Add(int Order_Total, int Order_Add, string VmName, int VmId)
{
    int CpuNeed = VmCategory[VmName][0];
    int MemoryNeed = VmCategory[VmName][1];
    int Mode = VmCategory[VmName][2];
    int flag = 0; // 变1为在目前的主机中找到合适的

    double RatioCnt = 50000000;

    int Cnt = HostBuy.size() * 0.01;
    //单节点模式
    if(Mode == 0)
    {
        int HostId = -1;
        int AorB = -1;
        int isFree = 1; //为0时代表还没有在开机状态中的主机中找到合适的

        //开始寻找有无合适主机
        for(int i = 0; i < HostBuy.size(); ++i)
        {
            int Cpu_A_CanBeDel = 0, Mem_A_CanBeDel = 0, Cpu_B_CanBeDel = 0, Mem_B_CanBeDel = 0;

            //算出总共可以被删除的
            for(auto del : OneDayDel_VC[HostBuy[i].HostId])
            {
                //del在这个之前的
                if(del.Order < Order_Total)
                {
                    Cpu_A_CanBeDel += del.Cpu_A;
                    Cpu_B_CanBeDel += del.Cpu_B;
                    Mem_A_CanBeDel += del.Mem_A;
                    Mem_B_CanBeDel += del.Mem_B;
                }
            }

            if((HostBuy[i].CoreNum_A + Cpu_A_CanBeDel) >= CpuNeed && (HostBuy[i].Memory_A + Mem_A_CanBeDel) >= MemoryNeed)
            {
                if((min(HostBuy[i].CoreNum_A + Cpu_A_CanBeDel - CpuNeed, HostBuy[i].Memory_A + Mem_A_CanBeDel - MemoryNeed)<p1&&
                    abs(HostBuy[i].CoreNum_A + Cpu_A_CanBeDel-CpuNeed-(HostBuy[i].Memory_A + Mem_A_CanBeDel-MemoryNeed))>p2))
                    continue;
                //double HostRatio =  (double)CpuNeed * MemoryNeed / (HostBuy[i].CoreNum_A + Cpu_A_CanBeDel) / (HostBuy[i].Memory_A + Mem_A_CanBeDel);
                double HostRatio = 0;
                if(HostBuy[i].CoreNum_A * 2 == HostBuy[i].CoreCapacity && HostBuy[i].CoreNum_B * 2 == HostBuy[i].CoreCapacity && isFree == 0)
                {
                    HostRatio = min((double)CpuNeed / (HostBuy[i].CoreNum_A), (double)MemoryNeed / (HostBuy[i].Memory_A ));
                }
                else
                {
                    HostRatio = min((double)CpuNeed / (HostBuy[i].CoreNum_A + Cpu_A_CanBeDel), (double)MemoryNeed / (HostBuy[i].Memory_A + Mem_A_CanBeDel));
                    if(isFree == 0)
                    {
                        isFree = 1;
                        RatioCnt = 500000000;
                    }
                }

                //找到更符合的ratio
                if((1.0 - HostRatio)< RatioCnt)
                {
                    RatioCnt = (1- HostRatio);
                    HostId = HostBuy[i].HostId;
                    AorB = 1;
                    --Cnt;
                    if(Cnt == 0) break;
                }
            }
            if((HostBuy[i].CoreNum_B + Cpu_B_CanBeDel) >= CpuNeed && (HostBuy[i].Memory_B + Mem_B_CanBeDel) >= MemoryNeed)
            {
                if((min(HostBuy[i].CoreNum_B + Cpu_B_CanBeDel-CpuNeed, HostBuy[i].Memory_B + Mem_B_CanBeDel-MemoryNeed)<p1&&
                    abs(HostBuy[i].CoreNum_B + Cpu_B_CanBeDel-CpuNeed-(HostBuy[i].Memory_B + Mem_B_CanBeDel-MemoryNeed))>p2))
                    continue;
                //double HostRatio =  (double)CpuNeed * MemoryNeed / (HostBuy[i].CoreNum_B + Cpu_B_CanBeDel) / (HostBuy[i].Memory_B + Mem_B_CanBeDel);
                double HostRatio = 0;
                if(HostBuy[i].CoreNum_A * 2 == HostBuy[i].CoreCapacity && HostBuy[i].CoreNum_B * 2 == HostBuy[i].CoreCapacity && isFree == 0)
                {
                    HostRatio = min((double)CpuNeed / (HostBuy[i].CoreNum_B), (double)MemoryNeed / (HostBuy[i].Memory_B ));
                }
                else
                {
                    HostRatio = min((double)CpuNeed / (HostBuy[i].CoreNum_B + Cpu_B_CanBeDel), (double)MemoryNeed / (HostBuy[i].Memory_B + Mem_B_CanBeDel));
                    if(isFree == 0)
                    {
                        isFree = 1;
                        RatioCnt = 500000000;
                    }
                }
                //找到更符合的ratio
                if((1- HostRatio)< RatioCnt)
                {
                    RatioCnt = (1- HostRatio);
                    HostId = HostBuy[i].HostId;
                    AorB = 2;
                    --Cnt;
                    if(Cnt == 0) break;
                }
            }
        }


        //在已有中，找到合适的了
        if(HostId != -1)
        {
            //开始执行部分删除操作
            for(vector<Request_Del>::iterator del = OneDayDel_VC[HostId].begin(); del != OneDayDel_VC[HostId].end(); )
            {
                //del在这个之前的
                if(del->Order < Order_Total && del->HostId == HostId)
                {
                    if(AorB == 1)
                    {
                        if(CpuNeed >= del->Cpu_A)
                        {
                            CpuNeed -= del->Cpu_A;
                            del->Cpu_A = 0;

                        }
                        else
                        {
                            del->Cpu_A -= CpuNeed;
                            CpuNeed = 0;
                        }

                        if(MemoryNeed >= del->Mem_A)
                        {

                            MemoryNeed -= del->Mem_A;
                            del->Mem_A = 0;
                        }
                        else
                        {
                            del->Mem_A -= MemoryNeed;
                            MemoryNeed = 0;
                        }
                    }
                    else if(AorB == 2)
                    {
                        if(CpuNeed >= del->Cpu_B)
                        {

                            CpuNeed -= del->Cpu_B;
                            del->Cpu_B = 0;
                        }
                        else
                        {
                            del->Cpu_B -= CpuNeed;
                            CpuNeed = 0;
                        }

                        if(MemoryNeed >= del->Mem_B)
                        {

                            MemoryNeed -= del->Mem_B;
                            del->Mem_B = 0;
                        }
                        else
                        {
                            del->Mem_B -= MemoryNeed;
                            MemoryNeed = 0;
                        }
                    }

                    if(del->Cpu_A == 0 && del->Cpu_B == 0 && del->Mem_A == 0 && del->Mem_B == 0)
                    {
                        DelVm_v2(del->VmId, 0, 0, 0, 0);
                        del = OneDayDel_VC[HostId].erase(del);
                    }
                    else
                        ++del;

                    if(MemoryNeed == 0 && CpuNeed == 0)
                        break;
                }
                else
                    ++del;
            }

            //记录虚拟机信息
            //AddVm(VmName, VmId, HostId, AorB);
            AddVm_v2(VmName, VmId, HostId, AorB, CpuNeed, MemoryNeed);
            flag = 1;
            //记录该部署操作
            DeployOp[Order_Add] = (make_pair(HostId, AorB));
        }
    }
        //双节点模式
    else if(Mode == 1)
    {
        int HostId = -1;

        int CpuNeed_A = CpuNeed / 2;
        int MemNeed_A = MemoryNeed / 2;
        int CpuNeed_B = CpuNeed / 2;
        int MemNeed_B = MemoryNeed / 2;

        //开始寻找有无合适主机
        for(int i = 0; i < HostBuy.size(); ++i)
        {
            int Cpu_A_CanBeDel = 0, Mem_A_CanBeDel = 0, Cpu_B_CanBeDel = 0, Mem_B_CanBeDel = 0;

            //算出总共可以被删除的
            for(auto del : OneDayDel_VC[HostBuy[i].HostId])
            {
                //del在这个之前的
                if(del.Order < Order_Total && del.HostId == HostBuy[i].HostId)
                {
                    Cpu_A_CanBeDel += del.Cpu_A;
                    Cpu_B_CanBeDel += del.Cpu_B;
                    Mem_A_CanBeDel += del.Mem_A;
                    Mem_B_CanBeDel += del.Mem_B;
                }
            }

            if((HostBuy[i].CoreNum_A + Cpu_A_CanBeDel) >= CpuNeed_A && (HostBuy[i].Memory_A + Mem_A_CanBeDel) >= MemNeed_A &&
               (HostBuy[i].CoreNum_B + Cpu_B_CanBeDel) >= CpuNeed_B && (HostBuy[i].Memory_B + Mem_B_CanBeDel) >= MemNeed_B)
            {
                if((min(HostBuy[i].CoreNum_A + Cpu_A_CanBeDel-CpuNeed_A, HostBuy[i].Memory_A + Mem_A_CanBeDel-MemNeed_A)<p1&&
                    abs(HostBuy[i].CoreNum_A + Cpu_A_CanBeDel-CpuNeed_A-(HostBuy[i].Memory_A + Mem_A_CanBeDel-MemNeed_A))>p2)||
                   (min(HostBuy[i].CoreNum_B + Cpu_B_CanBeDel-CpuNeed_B, HostBuy[i].Memory_B + Mem_B_CanBeDel-MemNeed_B)<p1&&
                    abs(HostBuy[i].CoreNum_B + Cpu_B_CanBeDel-CpuNeed_B-(HostBuy[i].Memory_B + Mem_B_CanBeDel-MemNeed_A))>p2))
                    continue;
                //double HostRatio = (double)CpuNeed_A * CpuNeed_B * MemNeed_A * MemNeed_B / (HostBuy[i].CoreNum_A + Cpu_A_CanBeDel)/ (HostBuy[i].CoreNum_B + Cpu_B_CanBeDel)/ (HostBuy[i].Memory_A + Mem_A_CanBeDel) / (HostBuy[i].Memory_B + Mem_B_CanBeDel);
                // double HostRatio = (double)(CpuNeed_A + CpuNeed_B) * (MemNeed_A + MemNeed_B) / (HostBuy[i].CoreNum_A + Cpu_A_CanBeDel)/ (HostBuy[i].CoreNum_B + Cpu_B_CanBeDel)/ (HostBuy[i].Memory_A + Mem_A_CanBeDel) / (HostBuy[i].Memory_B + Mem_B_CanBeDel);
                //double HostRatio = min(((double)(CpuNeed_A + CpuNeed_B) / (HostBuy[i].CoreNum_A + HostBuy[i].CoreNum_B + Cpu_A_CanBeDel+ Cpu_B_CanBeDel)), ((double)(MemNeed_A + MemNeed_B) / (HostBuy[i].Memory_A + Mem_A_CanBeDel+ HostBuy[i].Memory_B + Mem_B_CanBeDel)));
                double HostRatio1 = (double)CpuNeed_A / (HostBuy[i].CoreNum_A + Cpu_A_CanBeDel);
                double HostRatio2 = (double)CpuNeed_B / (HostBuy[i].CoreNum_B + Cpu_B_CanBeDel);
                double HostRatio3 = (double)MemNeed_A / (HostBuy[i].Memory_A + Mem_A_CanBeDel);
                double HostRatio4 = (double)MemNeed_B / (HostBuy[i].Memory_B + Mem_B_CanBeDel);
                double HostRatio = min(min(HostRatio1, HostRatio2), min(HostRatio3, HostRatio4));


//                double HostRatio = 0;
//                if(HostBuy[i].CoreNum_A * 2 == HostBuy[i].CoreCapacity && HostBuy[i].CoreNum_B * 2 == HostBuy[i].CoreCapacity && isFree == 0)
//                {
//                    double HostRatio1 = (double)CpuNeed_A / (HostBuy[i].CoreNum_A);
//                    double HostRatio2 = (double)CpuNeed_B / (HostBuy[i].CoreNum_B);
//                    double HostRatio3 = (double)MemNeed_A / (HostBuy[i].Memory_A);
//                    double HostRatio4 = (double)MemNeed_B / (HostBuy[i].Memory_B);
//                    HostRatio = min(min(HostRatio1, HostRatio2), min(HostRatio3, HostRatio4));
//                }
//                else
//                {
//                    double HostRatio1 = (double)CpuNeed_A / (HostBuy[i].CoreNum_A + Cpu_A_CanBeDel);
//                    double HostRatio2 = (double)CpuNeed_B / (HostBuy[i].CoreNum_B + Cpu_B_CanBeDel);
//                    double HostRatio3 = (double)MemNeed_A / (HostBuy[i].Memory_A + Mem_A_CanBeDel);
//                    double HostRatio4 = (double)MemNeed_B / (HostBuy[i].Memory_B + Mem_B_CanBeDel);
//                    HostRatio = min(min(HostRatio1, HostRatio2), min(HostRatio3, HostRatio4));
//                    if(isFree == 0)
//                    {
//                        isFree = 1;
//                        RatioCnt = 500000000;
//                    }
//                }

                //找到更符合的ratio
                if((1.0 - HostRatio)< RatioCnt)
                {
                    RatioCnt = (1- HostRatio);
                    HostId = HostBuy[i].HostId;
                    --Cnt;
                    if(Cnt == 0) break;
                }
            }
        }

        //在已有中，找到合适的了
        if(HostId != -1)
        {
            //开始执行部分删除操作
            for(vector<Request_Del>::iterator del = OneDayDel_VC[HostId].begin(); del != OneDayDel_VC[HostId].end(); )
            {
                //del在这个之前的
                if(del->Order < Order_Total && del->HostId == HostId)
                {

                    if(CpuNeed_A >= del->Cpu_A)
                    {
                        CpuNeed_A -= del->Cpu_A;
                        del->Cpu_A = 0;
                    }
                    else
                    {
                        del->Cpu_A -= CpuNeed_A;
                        CpuNeed_A = 0;
                    }

                    if(MemNeed_A >= del->Mem_A)
                    {

                        MemNeed_A -= del->Mem_A;
                        del->Mem_A = 0;
                    }
                    else
                    {
                        del->Mem_A -= MemNeed_A;
                        MemNeed_A = 0;
                    }

                    if(CpuNeed_B >= del->Cpu_B)
                    {

                        CpuNeed_B -= del->Cpu_B;
                        del->Cpu_B = 0;
                    }
                    else
                    {
                        del->Cpu_B -= CpuNeed_B;
                        CpuNeed_B = 0;
                    }

                    if(MemNeed_B >= del->Mem_B)
                    {

                        MemNeed_B -= del->Mem_B;
                        del->Mem_B = 0;
                    }
                    else
                    {
                        del->Mem_B -= MemNeed_B;
                        MemNeed_B = 0;
                    }

                    if(del->Cpu_A == 0 && del->Cpu_B == 0 && del->Mem_A == 0 && del->Mem_B == 0)
                    {
                        DelVm_v2(del->VmId, 0, 0, 0, 0);
                        del = OneDayDel_VC[HostId].erase(del);
                    }
                    else
                        ++del;

                    if(CpuNeed_A == 0 && CpuNeed_B == 0 && MemNeed_A == 0 && MemNeed_B == 0)
                        break;
                }
                else
                    ++del;
            }

            //记录虚拟机信息
            //AddVm(VmName, VmId, HostId, AorB);
            AddVm_v2(VmName, VmId, HostId, CpuNeed_A, MemNeed_A, CpuNeed_B, MemNeed_B);
            flag = 1;
            //记录该部署操作
            DeployOp[Order_Add] = (make_pair(HostId, 0));
        }
    }
    //无合适主机
    if(!flag)
    {
        int cpu = 0, mem = 0, count = 18;

        if(CpuNeedAll > CpuHave && MemNeedAll > MemHave)
        {
            cpu = CpuNeedAll - CpuHave;
            mem = MemNeedAll - MemHave;
            double rad  = 1.0 * cpu / mem;
            double rad1 = 1.0 * mem / cpu;
            for(auto &k : HostCate)
            {
                int value = min(k.CoreNum / rad + k.CoreNum, k.Memory / rad1 + k.Memory);
                k.myPrice = (1.0 * k.Prices * 1.01/ (DaysNum - Day) + k.EleCharge * 0.8) / (value+(k.CoreNum+k.Memory-value)*0.28);//*abs(1.0*k.CoreNum/k.Memory-rad);
                if(CpuNeedAll > MemNeedAll && k.CoreNum < k.Memory)
                    k.myPrice = 9999999;
                if(CpuNeedAll < MemNeedAll && k.CoreNum > k.Memory)
                    k.myPrice = 9999999;
            }
        }
        else
        {
            for(int k = OpCntofOneDay; k < OneDayAdd_VC.size() && k < OpCntofOneDay+count; k++)
            {
                cpu += OneDayAdd_VC[k].CpuNeed;
                mem += OneDayAdd_VC[k].Memory_Need;
            }
            double rad  = 1.0 * cpu / mem;
            double rad1 = 1.0 * mem / cpu;
            for(auto &k : HostCate)
            {
                int value = min(k.CoreNum / rad + k.CoreNum, k.Memory / rad1 + k.Memory);
                k.myPrice = (1.0 * k.Prices * 1.01/ (DaysNum - Day) + k.EleCharge * 0.8) / (value+(k.CoreNum+k.Memory-value)*0.28);//*abs(1.0*k.CoreNum/k.Memory-rad);
                if(CpuNeedAll > MemNeedAll && k.CoreNum < k.Memory)
                    k.myPrice = 9999999;
                if(CpuNeedAll < MemNeedAll && k.CoreNum > k.Memory)
                    k.myPrice = 9999999;
            }
        }

        sort(HostCate.begin(), HostCate.end());

        //单节点模式下购买主机
        if(Mode == 0)
        {
            for(int i = 0; i < HostCate.size(); ++i)
            {
                //找到合适的 开始买入
                if(HostCate[i].CoreNum >= 2 * CpuNeed && HostCate[i].Memory >= 2 * MemoryNeed)
                {
                    string HostName = HostCate[i].name;
                    int CoreCapacity = HostCate[i].CoreNum;
                    int MemoryCapacity = HostCate[i].Memory;
                    int CoreNum_A = HostCate[i].CoreNum / 2;
                    int Memory_A = HostCate[i].Memory / 2;
                    int CoreNum_B = HostCate[i].CoreNum / 2;
                    int Memory_B = HostCate[i].Memory / 2;
                    int EleCharge = HostCate[i].EleCharge;
                    BuyHost(HostName, HostIdCnt, CoreCapacity, MemoryCapacity, CoreNum_A, CoreNum_B, Memory_A, Memory_B, EleCharge);

                    //记录虚拟机信息
                    AddVm(VmName, VmId, HostIdCnt, 1);
                    //购买种类加1
                    if(OneDayBuyHost.count(HostCate[i].name) == 0)
                        PurchaseTypeNum++;

                    //记录该买入操作
                    OneDayBuyHost[HostCate[i].name]++;

                    //记录该部署操作
                    DeployOp[Order_Add] = (make_pair(HostIdCnt, 1));

                    ++HostIdCnt;

                    break;
                }
            }
        }
            //双节点模式下购买主机
        else
        {
            for(int i = 0; i < HostCate.size(); ++i)
            {
                //找到合适的 开始买入
                if(HostCate[i].CoreNum > CpuNeed && HostCate[i].Memory > MemoryNeed)
                {
                    string HostName = HostCate[i].name;
                    int CoreCapacity = HostCate[i].CoreNum;
                    int MemoryCapacity = HostCate[i].Memory;
                    int CoreNum_A = HostCate[i].CoreNum / 2;
                    int Memory_A = HostCate[i].Memory / 2;
                    int CoreNum_B = HostCate[i].CoreNum / 2;
                    int Memory_B = HostCate[i].Memory / 2;
                    int EleCharge = HostCate[i].EleCharge;
                    BuyHost(HostName, HostIdCnt, CoreCapacity, MemoryCapacity, CoreNum_A, CoreNum_B, Memory_A, Memory_B, EleCharge);
                    //记录虚拟机信息
                    AddVm(VmName, VmId, HostIdCnt);

                    //购买种类加1
                    if(OneDayBuyHost.count(HostCate[i].name) == 0)
                        PurchaseTypeNum++;

                    //记录该买入操作
                    OneDayBuyHost[HostCate[i].name]++;

                    //记录该部署操作
                    DeployOp[Order_Add] = (make_pair(HostIdCnt, 0));

                    ++HostIdCnt;

                    break;
                }
            }
        }
    }
}

void Mig()
{
    struct Re_Level
    {
        int Cpu;
        int Mem;
        bool operator < (const Re_Level &a) const
        {
            return Cpu + Mem > a.Cpu + a.Mem;
        }
    };
    set<Re_Level> Level_S;
    set<Re_Level> Level_D;

    //0为第一轮，要考虑能否迁移；  1 为第二轮，能迁移，就迁移；  2为满了或者没有可以迁移的了，退出；
    int Flag = 1;
    while (Flag == 0 || Flag == 1)
    {

        //迁移操作
        for (int right = HostBuy.size() - 1; right > 0; --right)
        {

            //该主机是空的     跳过
            if ((HostBuy[right].CoreCapacity == HostBuy[right].CoreNum_A * 2) && (HostBuy[right].CoreCapacity == HostBuy[right].CoreNum_B * 2))
                continue;

            //分别是存入 下标left，cpu，memory， vmid, 要去放的主机的AB, 原始主机的AB
            vector<vector<int>> OneHostMig;


            for (auto i = HostBuy[right].VmOnThisHost.begin(); i != HostBuy[right].VmOnThisHost.end(); ++i)
            {
                //迁移次数用完了
                if(Migration_Num >= (3 * VmNum / 100 ))
                {
                    Flag = 2;
                    break;
                }

                int VmID = *i;

                //被迁徙过 跳过
                if(AlreadyMigVm.count(VmID))
                    continue;

                string VmName = VmDeploy[VmID].VmName;

                int AorB_right = VmDeploy[VmID].AB;    //0是双节点 1放在A 2 放在B节点
                int CpuNeed = VmCategory[VmName][0];
                int MemoryNeed = VmCategory[VmName][1];

                int flagflag = 0;

                vector<Re_Level> bank;
                int flagggg = 0;
                //超过资源限制 这不去寻找了
                if(AorB_right == 0)
                {
                    for(auto i : Level_D)
                    {
                        if(CpuNeed >= i.Cpu && MemoryNeed >= i.Mem)
                        {
                            flagggg = 1;
                            break;
                        }
                        if(CpuNeed < i.Cpu && MemoryNeed < i.Mem)
                            bank.push_back(i);
                    }
                }
                else
                {
                    for(auto i : Level_S)
                    {
                        if(CpuNeed >= i.Cpu && MemoryNeed >= i.Mem)
                        {
                            flagggg = 1;
                            break;
                        }
                        if(CpuNeed < i.Cpu && MemoryNeed < i.Mem)
                            bank.push_back(i);
                    }
                }

                if(flagggg == 1) continue;

                double RatioCnt = 50000000;

                int HostId = -1;
                int AorB_left = -1;
                int left_cnt = -1;

                //这个变量定义在left的多少个主机里面放比例最合适
                double Num_Cnt_S = 0.3 * HostBuy.size();
                double Num_Cnt_D = 0.15 * HostBuy.size();
                //开始在其他主机上查找
                for (int left = 0; left < right; ++left)
                {
                    //单节点
                    if(AorB_right > 0)
                    {
                        if (HostBuy[left].CoreNum_A >= CpuNeed && HostBuy[left].Memory_A >= MemoryNeed)
                        {
                            if (min(HostBuy[left].CoreNum_A - CpuNeed, HostBuy[left].Memory_A - MemoryNeed) <p1 &&
                                abs(HostBuy[left].CoreNum_A - CpuNeed - (HostBuy[left].Memory_A - MemoryNeed)) > p2)
                                continue;

                            --Num_Cnt_S;
                            //double HostRatio =  (double)CpuNeed * MemoryNeed / HostBuy[left].CoreNum_A / HostBuy[left].Memory_A;
                            double HostRatio = min((double)CpuNeed / HostBuy[left].CoreNum_A, (double)MemoryNeed / HostBuy[left].Memory_A);
                            //找到更符合的ratio
                            if((1- HostRatio)< RatioCnt)
                            {
                                RatioCnt = (1- HostRatio);
                                HostId = HostBuy[left].HostId;
                                AorB_left = 1;
                                left_cnt = left;
                            }
                        }
                        if (HostBuy[left].CoreNum_B >= CpuNeed && HostBuy[left].Memory_B >= MemoryNeed)
                        {
                            if(min(HostBuy[left].CoreNum_B-CpuNeed,HostBuy[left].Memory_B - MemoryNeed)<p1&&
                               abs(HostBuy[left].CoreNum_B-CpuNeed-(HostBuy[left].Memory_B - MemoryNeed))>p2)
                                continue;

                            --Num_Cnt_S;
                            //double HostRatio =  (double)CpuNeed * MemoryNeed / HostBuy[left].CoreNum_B / HostBuy[left].Memory_B;
                            double HostRatio = min((double)CpuNeed / HostBuy[left].CoreNum_B, (double)MemoryNeed / HostBuy[left].Memory_B);
                            //找到更符合的ratio
                            if((1- HostRatio)< RatioCnt)
                            {
                                RatioCnt = (1- HostRatio);
                                HostId = HostBuy[left].HostId;
                                AorB_left = 2;
                                left_cnt = left;
                            }
                        }
                    }
                        //双节点
                    else
                    {
                        if ((HostBuy[left].CoreNum_A >= CpuNeed / 2) && (HostBuy[left].Memory_A >= MemoryNeed / 2) && (HostBuy[left].CoreNum_B >= CpuNeed / 2) && (HostBuy[left].Memory_B >= MemoryNeed / 2))
                        {
                            if((min(HostBuy[left].CoreNum_A - CpuNeed / 2,HostBuy[left].Memory_A - MemoryNeed / 2)<p1&&
                                abs(HostBuy[left].CoreNum_A - CpuNeed / 2-(HostBuy[left].Memory_A - MemoryNeed / 2))>p2)||
                               (min(HostBuy[left].CoreNum_B - CpuNeed / 2,HostBuy[left].Memory_B - MemoryNeed / 2)<p1&&
                                abs(HostBuy[left].CoreNum_B - CpuNeed / 2-(HostBuy[left].Memory_B - MemoryNeed / 2))>p2))
                                continue;
                            --Num_Cnt_D;
                            double HostRatio =  0.125 * CpuNeed * CpuNeed * MemoryNeed * MemoryNeed / HostBuy[left].CoreNum_A / HostBuy[left].CoreNum_B / HostBuy[left].Memory_A / HostBuy[left].Memory_B;

                            if((1- HostRatio)< RatioCnt)
                            {
                                RatioCnt = (1- HostRatio);
                                HostId = HostBuy[left].HostId;
                                AorB_left = 0;
                                left_cnt = left;
                            }
                        }
                    }
                    //if(Num_Cnt_S <= 0 || Num_Cnt_D <= 0) break;
                }

                //找到合适的了
                if(HostId != -1)
                {
                    //以前被迁移过
                    if(AlreadyMigVm.count(VmID))
                    {
                        int index = 0;
                        for(; index < MigrationOp.size(); ++index)
                            if(MigrationOp[index][0] == VmID);
                        break;
                        MigrationOp.erase(MigrationOp.begin() + index);
                    }
                    else
                    {
                        AlreadyMigVm.insert(VmID);
                        Migration_Num++;
                    }
                    //记录本次迁移操作

                    //在新主机上写入信息
                    if(AorB_left == 0)
                    {
                        HostBuy[left_cnt].CoreNum_A -= CpuNeed / 2;
                        HostBuy[left_cnt].Memory_A -= MemoryNeed / 2;
                        HostBuy[left_cnt].CoreNum_B -= CpuNeed / 2;
                        HostBuy[left_cnt].Memory_B -= MemoryNeed / 2;
                    }
                    else if(AorB_left == 1)
                    {
                        HostBuy[left_cnt].CoreNum_A -= CpuNeed;
                        HostBuy[left_cnt].Memory_A -= MemoryNeed;
                    }
                    else if(AorB_left == 2)
                    {
                        HostBuy[left_cnt].CoreNum_B -= CpuNeed;
                        HostBuy[left_cnt].Memory_B -= MemoryNeed;
                    }

                    HostBuy[left_cnt].VmOnThisHost.insert(VmID);

                    OneHostMig.push_back(vector<int>{left_cnt, CpuNeed, MemoryNeed, VmID, AorB_left, AorB_right});
                    flagflag = 1;
                }
                else
                {
                    if(AorB_right == 0)
                    {
                        for(auto i : bank)
                            Level_D.erase(i);
                        Re_Level tmp;
                        tmp.Cpu = CpuNeed;
                        tmp.Mem = MemoryNeed;
                        Level_D.insert(tmp);
                    }
                    else
                    {
                        for(auto i : bank)
                            Level_S.erase(i);
                        Re_Level tmp;
                        tmp.Cpu = CpuNeed;
                        tmp.Mem = MemoryNeed;
                        Level_S.insert(tmp);
                    }
                }


                if(flagflag == 0 && Flag == 0) break;
            }


            //开始判断
            //第二轮迁移 或者已经次数用完了
            if(Flag == 1 || Flag == 2)
            {
                for(auto mi : OneHostMig)
                {
                    int left = mi[0];
                    int CpuNeed = mi[1];
                    int MemoryNeed = mi[2];
                    int VmId = mi[3];
                    int AorB_left = mi[4];
                    int AorB_right = mi[5];

                    //删除 和 添加
                    HostBuy[right].VmOnThisHost.erase(VmId);
                    HostBuy[left].VmOnThisHost.insert(VmId);

                    //修改部署了的虚拟机的信息
                    VmDeploy[VmId].AB = AorB_left;
                    VmDeploy[VmId].HostId = HostBuy[left].HostId;

                    //记录本次迁移操作
                    MigrationOp.push_back(vector<int>{VmId, HostBuy[left].HostId, AorB_left});

                    //双节点
                    if(AorB_right == 0)
                    {
                        //删除原主机上的信息
                        HostBuy[right].CoreNum_A += CpuNeed / 2;
                        HostBuy[right].Memory_A += MemoryNeed / 2;
                        HostBuy[right].CoreNum_B += CpuNeed / 2;
                        HostBuy[right].Memory_B += MemoryNeed / 2;

                    }
                    else if(AorB_right == 1)
                    {
                        HostBuy[right].CoreNum_A += CpuNeed;
                        HostBuy[right].Memory_A += MemoryNeed;
                    }
                    else if(AorB_right == 2)
                    {
                        HostBuy[right].CoreNum_B += CpuNeed;
                        HostBuy[right].Memory_B += MemoryNeed;
                    }
                }

                if(Flag == 2) break;
            }

            //第一轮迁移
            if(Flag == 0)
            {
                //本主机迁移完了，或者说次数已经用完了，则对这台主机开始迁移
                if(HostBuy[right].VmOnThisHost.size() == OneHostMig.size())
                {
                    for(auto mi : OneHostMig)
                    {
                        int left = mi[0];
                        int CpuNeed = mi[1];
                        int MemoryNeed = mi[2];
                        int VmId = mi[3];
                        int AorB_left = mi[4];
                        int AorB_right = mi[5];

                        //删除 和 添加

                        HostBuy[right].VmOnThisHost.erase(VmId);
                        HostBuy[left].VmOnThisHost.insert(VmId);

                        //修改部署了的虚拟机的信息
                        VmDeploy[VmId].AB = AorB_left;
                        VmDeploy[VmId].HostId = HostBuy[left].HostId;

                        //记录本次迁移操作
                        MigrationOp.push_back(vector<int>{VmId, HostBuy[left].HostId, AorB_left});

                        //双节点
                        if(AorB_right == 0)
                        {
                            //删除原主机上的信息
                            HostBuy[right].CoreNum_A += CpuNeed / 2;
                            HostBuy[right].Memory_A += MemoryNeed / 2;
                            HostBuy[right].CoreNum_B += CpuNeed / 2;
                            HostBuy[right].Memory_B += MemoryNeed / 2;

                        }
                        else if(AorB_right == 1)
                        {
                            HostBuy[right].CoreNum_A += CpuNeed;
                            HostBuy[right].Memory_A += MemoryNeed;
                        }
                        else if(AorB_right == 2)
                        {
                            HostBuy[right].CoreNum_B += CpuNeed;
                            HostBuy[right].Memory_B += MemoryNeed;
                        }
                    }
                }
                    //没有迁移完同时次数也没用完
                else
                {
                    Migration_Num -= OneHostMig.size();

                    //复原left主机被占用了的
                    for(auto mi : OneHostMig)
                    {
                        int left = mi[0];
                        int CpuNeed = mi[1];
                        int MemoryNeed = mi[2];
                        int VmId = mi[3];
                        int AorB_left = mi[4];
                        int AorB_right = mi[5];

                        HostBuy[left].VmOnThisHost.erase(VmId);

                        if(AorB_left == 0)
                        {
                            HostBuy[left].CoreNum_A += CpuNeed / 2;
                            HostBuy[left].Memory_A += MemoryNeed / 2;
                            HostBuy[left].CoreNum_B += CpuNeed / 2;
                            HostBuy[left].Memory_B += MemoryNeed / 2;
                            HostBuy[left].VmOnThisHost.erase(VmId);
                        }
                        else if(AorB_left == 1)
                        {
                            HostBuy[left].CoreNum_A += CpuNeed;
                            HostBuy[left].Memory_A += MemoryNeed;
                            HostBuy[left].VmOnThisHost.erase(VmId);
                        }
                        else if(AorB_left == 2)
                        {
                            HostBuy[left].CoreNum_B += CpuNeed;
                            HostBuy[left].Memory_B += MemoryNeed;
                            HostBuy[left].VmOnThisHost.erase(VmId);
                        }

                    }
                }
            }
            OneHostMig.clear();
        }
        ++Flag;
    }
}

//读取一天的数据
void ReadOneDayData(int i)
{
    string RequestOp, RequestVmName, RequestVmId;

    int tmpOpNum = 0; //当天的操作次数
    scanf("%d",&tmpOpNum);
    DayOp[i].first = tmpOpNum;

    for(int j = 0; j < tmpOpNum; ++j)
    {
        cin >> RequestOp;
        if(RequestOp[1] == 'a')
        {
            cin >> RequestVmName >> RequestVmId;
            SaveOp(i, RequestOp, RequestVmName, RequestVmId);
        }
        else
        {
            cin >> RequestVmId;
            SaveOp(i, RequestOp, RequestVmId);
        }
    }
}

//读取数据
void ReadData()
{
    //主机种类数量
    int HostNum = 0;
    scanf("%d", &HostNum);
    //读入主机
    string HostName, HostCoreNum, HostMemory, HostPrices, HostEleCharge;
    for(int i =0; i < HostNum; ++i)
    {
        cin >> HostName >> HostCoreNum >> HostMemory >> HostPrices >> HostEleCharge;
        SaveHost(HostName, HostCoreNum, HostMemory, HostPrices, HostEleCharge);
    }

    //虚拟机种类数量
    int VmTypeNum = 0;
    scanf("%d", &VmTypeNum);

    //读入虚拟机
    string VmName, VmCoreNum, VmMemory, VmModes;
    for(int i =0; i < VmTypeNum; ++i)
    {
        cin >> VmName >> VmCoreNum >> VmMemory >> VmModes;
        SaveVm(VmName, VmCoreNum, VmMemory, VmModes);
    }

    //天数
    scanf("%d",&DaysNum);

    //K
    scanf("%d",&K);

    DayOp.resize(DaysNum);

    //读入该天具体操作
    for(int i = 0; i < K; ++i)
    {
        ReadOneDayData(i);
    }
}

void OutputRes()
{
    //开始输出结果
    cout << "(purchase, " << PurchaseTypeNum << ")\n";
    for (auto i : OneDayBuyHost) {
        cout << "(" << i.first << ", " << i.second << ")\n";
    }

    cout << "(migration, "<<Migration_Num<<")"<<endl;
    for(auto i : MigrationOp)
    {
        int HostID = HostIdMapping[i[1]];
        if(i[2] == 0)      cout << "(" << to_string(i[0]) << ", "<<to_string(HostID)<<")"<<endl;
        else if(i[2] == 1) cout << "(" << to_string(i[0]) << ", "<<to_string(HostID)<<", "<<"A)"<<endl;
        else if(i[2] == 2) cout << "(" << to_string(i[0]) << ", "<<to_string(HostID)<<", "<<"B)"<<endl;
    }

    for (auto i : DeployOp)
    {
        int HostID = HostIdMapping[i.first];
        if(i.second == 0) cout << "(" << to_string(HostID) <<")"<<endl;
        else if(i.second == 1) cout << "(" << to_string(HostID) <<", A)"<<endl;
        else if(i.second == 2) cout << "(" << to_string(HostID) <<", B)"<<endl;
    }
}

void MappingHostId()
{
    int cnt = HostIdCntForMapping;
    for(auto i : OneDayBuyHost)
    {
        int k = i.second;
        string name = i.first;
        for(int i = HostIdCntForMapping; i < HostBuy.size(); ++i)
        {
            if(HostBuy[i].HostName == name)
            {
                HostIdMapping[HostBuy[i].HostId] = cnt;
                ++cnt;
                --k;
            }
            if(!k) break;
        }
    }
}

void Init(int i)
{

    PurchaseTypeNum = 0;
    Migration_Num = 0;
    OneDayBuyHost.clear();
    DeployOp.clear();
    MigrationOp.clear();
    AlreadyMigVm.clear();
    HostIdCntForMapping = HostIdCnt;
    OneDayAdd_VC.clear();
    OneDayDel_VC.clear();
}

void Work()
{
    for(int i = 0; i < DaysNum; ++i)
    {
        //每天初始化
        Init(i);

        //对主机进行排序
        sort(HostBuy.begin(), HostBuy.end()); //先排序

        //迁徙
        Mig();

        //记录当天有多少次add请求
        int AddNum_CNT = 0;
        //记录当天有多少次del请求
        int DelNum_CNT = 0;

        //分别记录下 该天的Add和Del请求
        for(int j = 0; j < DayOp[i].first; ++j)
        {
            if(DayOp[i].second[j][0] == "add")
            {
                Request_Add tmp;
                tmp.Order = AddNum_CNT + DelNum_CNT;
                tmp.Order_Add = AddNum_CNT;
                tmp.VmName      = DayOp[i].second[j][1];
                tmp.VmId        = stoi(DayOp[i].second[j][2]);
                tmp.CpuNeed     = VmCategory[tmp.VmName][0];
                tmp.Memory_Need = VmCategory[tmp.VmName][1];
                tmp.Mode        = VmCategory[tmp.VmName][2];

                OneDayAdd_VC.push_back(tmp);

                ++AddNum_CNT;
            }
            else
            {
                Request_Del tmp;
                tmp.Order = AddNum_CNT + DelNum_CNT;
                tmp.Order_Del = DelNum_CNT;
                tmp.VmId        = stoi(DayOp[i].second[j][1]);
                tmp.flag = 0;
                tmp.HostId = VmDeploy[tmp.VmId].HostId;
                string VmName = VmDeploy[tmp.VmId].VmName;
                int AorB = VmDeploy[tmp.VmId].AB;
                int CpuNeed = VmCategory[VmName][0];
                int MemNeed = VmCategory[VmName][1];
                if(AorB == 0)
                {
                    tmp.Cpu_A = CpuNeed / 2;
                    tmp.Cpu_B = CpuNeed / 2;
                    tmp.Mem_A = MemNeed / 2;
                    tmp.Mem_B = MemNeed / 2;
                }
                else if(AorB == 1)
                {
                    tmp.Cpu_A = CpuNeed;
                    tmp.Mem_A = MemNeed;
                    tmp.Cpu_B = 0;
                    tmp.Mem_B = 0;
                }
                else if(AorB == 2)
                {
                    tmp.Cpu_B = CpuNeed;
                    tmp.Mem_B = MemNeed;
                    tmp.Cpu_A = 0;
                    tmp.Mem_A = 0;
                }

                OneDayDel_VC[tmp.HostId].push_back(tmp);

                ++DelNum_CNT;
            }
        }

        //对该天的Add操作排序
        sort(OneDayAdd_VC.begin(), OneDayAdd_VC.end());

        //定义该天部署操作的容量
        DeployOp.resize(AddNum_CNT);

        //对Del请求排序，Order顺序靠后的放在前面
        for(auto &j : OneDayDel_VC)
            sort(j.second.begin(), j.second.end());

        //处理该天的Add操作
        for(int j = 0; j < OneDayAdd_VC.size(); ++j)
        {
            OpCntofOneDay = j;
            Add(OneDayAdd_VC[j].Order, OneDayAdd_VC[j].Order_Add, OneDayAdd_VC[j].VmName, OneDayAdd_VC[j].VmId);
        }

        //处理该天的Del操作
        for(auto j : OneDayDel_VC)
        {
            for(auto k : j.second)
                DelVm_v2(k.VmId, k.Cpu_A, k.Mem_A, k.Cpu_B, k.Mem_B);
        }

        MappingHostId();

        OutputRes();

        ++Day;

        if(i + K < DaysNum)
            ReadOneDayData(i + K);
    }
}
/*******************************************************/




/***********************主函数********************************/
int main()
{
//    const string filePath = "../training-1.txt";
//    std::freopen(filePath.c_str(),"rb",stdin);

    ReadData();
    Work();
    return 0;
}