
1.客户端和服务端可以实现数据互发

2.假如将来多个客户端发送数据，我们可以用队列来避免使用线程
  通过队列保存数据和客户信息，取数据或者使用数据的地方只要
  通过区分客户信息来分流数据就可以了，这样可以保证数据有很
  强的顺序性和实时性
  本来想使用链表或者ringbuf来存所有信息但是这样好处是可以随
  意操作节点，但是效率低。
  
3.用户发来的数据目前使用纯字符串，没有插入json待需要再做考虑，或者结构体转换

4.对于KCP的传输模式待做细分，准确划分

5.KCP的所有涉及大小的参数待同一

6.经过测试发现有些数据在KCP入队和出对会在KCP自身程序出错，但
经过测试发现有些错误是协议传输过程中自身设定的，非常规错误。

  
