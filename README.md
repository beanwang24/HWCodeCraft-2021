## HWCodeCraft-2021

- **部署**

将每一天所有的请求按需求资源降序排序，先处理资源需求大的Add请求。同时为了执行Add请求时，利用改天中之前的Del请求，会先去寻找在这个Add请求之前的Del请求。因此存储Add和Del请求时，需要存储该请求在该天中的顺序。

```c++
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
    
    //该主机资源满足所需条件
    if(...){...}
}
```

遍历所有的主机，记录满足资源的主机的HostRatio，利用木桶效应记录Cpu和内存放置后的使用率的较小者，在利用率最大的主机中放入改虚拟机。

```c++
HostRatio = min((double)CpuNeed / (HostBuy[i].CoreNum_A), (double)MemoryNeed / (HostBuy[i].Memory_A ));
```

