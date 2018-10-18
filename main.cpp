#include "threads.cpp"

void MyExampleThread1() {
  std::cout << "B" << std::endl;
  mythread_yield();
  std::cout << "E" << std::endl;
  mythread_exit();
}

void MyExampleThread2() {
  std::cout << "D" << std::endl;
  mythread_yield();
  std::cout << "H" << std::endl;
  mythread_exit();
}

int main() {
  mythread_init();
  std::cout << "A" << std::endl;
  int thread1 = mythread_fork();
  if (thread1==0)
    MyExampleThread1();
  std::cout << "C" << std::endl;
  int thread2 = mythread_fork();
  if (thread2==0)
    MyExampleThread2();
  std::cout << "F" << std::endl;
  mythread_join(thread1);
  std::cout << "G" << std::endl;
  mythread_join(thread2);
  std::cout << "I" << std::endl;
}
