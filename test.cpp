#include "threads.cpp"

void MyExampleThread1() {
  std::cout << "B " << current_thread->id << std::endl;
  mythread_yield();
  std::cout << "E " << current_thread->id << std::endl;
  for (int i = 0; i < 1000000000; ++i);
  for (int i = 0; i < 1000000000; ++i);
  mythread_exit();
}

void MyExampleThread2() {
  for (int i = 0; i < 1000000000; ++i);
  for (int i = 0; i < 1000000000; ++i);
  std::cout << "D " << current_thread->id << std::endl;
  mythread_yield();
  std::cout << "H " << current_thread->id << std::endl;
  mythread_exit();
}

int main() {
  mythread_init();
  std::cout << "A " << current_thread->id << std::endl;
  int thread1 = mythread_fork();
  if (thread1==0)
    MyExampleThread1();
  std::cout << "C " << current_thread->id << std::endl;
  int thread2 = mythread_fork();
  for (int i = 0; i < 1000000000; ++i);
  for (int i = 0; i < 1000000000; ++i);
  if (thread2==0)
    MyExampleThread2();
  for (int i = 0; i < 1000000000; ++i);
  for (int i = 0; i < 1000000000; ++i);
  std::cout << "F " << current_thread->id << std::endl;
  mythread_join(thread1);
  std::cout << "G " << current_thread->id << std::endl;
  mythread_join(thread2);
  std::cout << "I " << current_thread->id << std::endl;
}