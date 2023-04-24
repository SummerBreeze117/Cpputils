# Cpputils

### BlockingQueue

阻塞队列，使用 `std::deque`，`std::condition_variable` ，`std::mutex` 实现。

阻塞读写的情况

```cpp
BlockingQueue<int> q;

q.push(x); // 如果队列满，线程会阻塞等待

int element;
q.pop(element); // 如果队列空，线程会阻塞等待
```

非阻塞读写，立即返回

```cpp
bool status_1 = q.try_push(x); // 如果队列满，status_1 == false

int element;
bool status_2 = q.try_pop(element); // 如果队列空，status_2 == false
```


### LockFreeQueue

有界无锁队列，通过 `std::atomic` 读写指针 + 循环缓冲区实现。

```cpp
LockFreeQueue<int> q;

bool status_1 = q.try_push(x); // 如果队列满, status_1 == false
while (!q.try_push(x)); // busy-loop直到队列有空间可写

int element;
bool status_2 = q.try_pop(element); // 如果队列空，status_2 == false
while (!q.try_pop(element)); // busy-loop直到队列有数据可读
```


### TimerClock


```cpp
TimerClock tc; // checkpoint 0
//...
// codeblock 0
//...
double used_time_0 = tc.getTimerMilliSec(); // t1 - t0
tc.update(); // checkpoint 1(update tc)
//...
// codeblock 1
//...
double used_time_1 = tc.getTimerMilliSec(); // t2 - t1
```
