# HWCodeCraft-2021

- ## 购买

每次购买时，对主机种类进行排序，买排序后第一个能放下的主机。排序规则如下。

会记录已知的（包括未来K天）一共所需要的Cpu和内存资源：CpuNeedAll，CpuNeedAll，和已买的主机拥有的Cpu和内存资源：CpuHave，MemHave。

```C++
//K天后一共需要的大于现在已有的 
if(CpuNeedAll > CpuHave && CpuNeedAll > MemHave)
{
	//按照 cpu = CpuNeedAll - CpuHave，mem = MemNeedAll - MemHave;作为性价比计算因子之一。计算性价比
}
else
{
    	//记录接下来几次Add请求加起来所需要的的cpu和mem，仿照上面，计算性价比。
}
```



- ## **部署**

将每一天所有的请求按需求资源降序排序，先处理资源需求大的Add请求。同时为了执行Add请求时，利用该天中之前的Del请求，会先去寻找在这个Add请求之前的Del请求。因此存储Add和Del请求时，需要存储该请求在该天中的顺序。

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



- ## 迁移

  每天迁移之前最已购买主机按照空闲率升序排序，从后往前迁移。计算空闲率时也使用木桶效应，记录CPU和内存二者空闲率的大者。

  ```c++
      bool operator < (const Host &a)
      {
          double this_notUseRatio = max((double) (CoreNum_A + CoreNum_B) / CoreCapacity, (double)(Memory_A + Memory_B) / MemoryCapacity);
          double a_notUseRatio    = max((double) (a.CoreNum_A + a.CoreNum_B) / a.CoreCapacity, (double)(a.Memory_A + a.Memory_B) / a.MemoryCapacity);
          return this_notUseRatio < a_notUseRatio;
      }
  ```

  从后往前迁移时，会遍历前面的所有主机，仿照部署时的操作，选择放置后的主机利用率最大的主机进行迁移。
  
  
  
- ## 总结

软挑前前后后搞了一个月，现在回想起来还是很有意思的，团队里面三个人都是跨专业第一次参加软挑，走了不少弯路，写了不少bug。不过大家为了最开始的目标：进复赛，可以说都拼了老命，QAQ。初赛和复赛最后全靠调参取得了初赛第10，复赛第7的成绩，虽说没进决赛有点小遗憾，但总体大家都还是比较满意的，算是没有浪费这一个月的努力，再次感谢我的队友们。
