# threadpool
based on C++11, a mini `threadpool`, accept variable number of parameters.  
基于C++11的线程池，简洁且可以带任意多的参数  

管理一个任务队列，一个线程队列，然后每次取一个任务分配给一个线程去做，循环往复。  
有意思的是，限制只创建一个线程，这样就是一个完全的任务队列了。  

线程池，可以提交变参函数或lambda表达式的匿名函数执行，可以获取执行返回值。  