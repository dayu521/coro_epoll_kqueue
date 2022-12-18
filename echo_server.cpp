#include "io_context.h"
#include "awaiters.h"

task<bool> inside_loop(Socket &socket) {
  char buffer[1024] = {0};
  ssize_t recv_len = co_await socket.recv(buffer, sizeof(buffer));
  ssize_t send_len = 0;
  while (send_len < recv_len) {
    ssize_t res = co_await socket.send(buffer + send_len, recv_len - send_len);
    if (res <= 0) {
      co_return false;
    }
    send_len += res;
  }

  std::cout << "Done send " << send_len << "\n";
  if (recv_len <= 0) {
    co_return false;
  }
  printf("%s\n", buffer);
  co_return true;
}

task2 echo_socket(std::shared_ptr<Socket> socket) {
  for (;;) {
    std::cout << "BEGIN\n";
    bool b = co_await inside_loop(*socket);
    if (!b)
      break;
    std::cout << "END\n";
  }
}

task<> accept(Socket &listen) {
  for (;;) {
    echo_socket(co_await listen.accept());
    // auto socket = co_await listen.accept();
    // auto t = echo_socket(socket);
    // t.resume();
  }
}

/*
    promise.get_return_object()

 co_await promise.initial_suspend()
        std::suspend_always
        suspend_never
co_return,
    无返回值时. 如果没有 Promise::return_void() 成员函数，那么则行为未定义
        promise.return_void()
    有非void返回值
        promise.return_value(expr)
    promise.final_suspend() 并 co_await 其结果
如果协程因未捕捉的异常结束，那么它进行下列操作：
    捕捉异常并在 catch 块内调用 promise.unhandled_exception()
当经由 co_return
或未捕捉异常而终止协程导致协程状态被销毁，或经由其句柄而导致其被销毁时，它进行下列操作：
    调用承诺对象的析构函数。
    调用各个函数形参副本的析构函数。
    调用 operator delete 以释放协程状态所用的内存。
    转移执行回到调用方/恢复方。

co_await
由可等待体返回等待器对象,等待器对象有三个方法
    awaiter.await_ready()
    awaiter.await_suspend(handle) 参数handel是外部调用者协程
    awaiter.await_resume()
这三个方法在co_await表达式中在合适的时机被调用

co_yield 等价于 co_await promise.yield_value(表达式)
*/

int main() {
  IoContext io_context;
  Socket listen{"10009", io_context};
  auto t = accept(listen);
  t.resume();

  io_context.run(); // 启动事件循环
}
