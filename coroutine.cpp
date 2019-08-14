#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
using namespace std;

#define co_begin \
if (completed)\
{\
  return; \
}\
\
if (child_coroutine)\
{\
  child_coroutine->resume();\
\
  if (child_coroutine->is_completed())\
  {\
  child_coroutine = nullptr;\
  }\
  else\
  {\
    return;\
  }\
}\
switch (continue_line)\
{\
case 0:

#define await(operation) \
continue_line = __LINE__; \
child_coroutine = &operation; \
setupChild(); \
operation.resume(); \
if (!operation.is_completed())\
{ \
  printf("suspend by await\n");\
  return; \
} \
case __LINE__:

#define suspend \
continue_line = __LINE__;\
printf("suspend\n");\
return;\
case __LINE__:

#define co_return \
completed = true; \
return;

#define co_end \
} \
completed = true;

class co_executor;

class coroutine_base
{
  friend class co_executor;
  virtual void start() = 0;
  virtual void resume() = 0;
  virtual bool is_completed() = 0;
  virtual void reset() = 0;
protected:
  virtual void main() = 0;
  virtual void set_executor(co_executor* executor) = 0;
};

class co_executor
{
private:
  coroutine_base* m_co;
public:
  co_executor(coroutine_base* co)
  {
    m_co = co;
    m_co->set_executor(this);
  }

  void start()
  {
    m_co->start();
  }
  
  void resume()
  {
    m_co->resume();
  }
  
  void reset()
  {
    m_co->reset();
  }
};


class coroutine: public coroutine_base
{
public:
  coroutine()
    : continue_line(0), completed(false), child_coroutine(nullptr), executor(nullptr)
  {
  }
  
  virtual void start()
  {
    if (continue_line == 0)
    {
      main();
    }
  }
  
  virtual void resume()
  {
    if (!completed)
    {
      main();
    }
  }
  
  virtual void reset()
  {
    continue_line = 0;
    completed = false;
    child_coroutine = nullptr;
  }
  
  virtual bool is_completed()
  {
    return completed;
  }
  
protected:

  bool         completed;
  int          continue_line;
  coroutine*   child_coroutine;
  co_executor* executor;

  virtual void main()
  {
  }
  
  virtual void set_executor(co_executor* _executor)
  {
    executor = _executor;
  }
  
  virtual void setupChild()
  {
    child_coroutine->set_executor(executor);
  }
};

/* Test */

std::function<void()> read_completed;

class co_test_inner1: public coroutine
{
public:
    int result;

  virtual void main()
  {
    printf("    start/resume co_test_inner1\n");
    co_begin
    printf("    co_test_inner1: step 1!\n");
    read_completed = [this]{
      executor->resume();
    };
    suspend;
    printf("    co_test_inner1: step 2!\n");
    result = 33;
    co_end
  }
};

class co_test_inner2: public coroutine
{
public:
    int result;

  virtual void main()
  {
    printf("    start/resume co_test_inner2\n");
    co_begin
    printf("    co_test_inner2: step 1!\n");
    suspend
    printf("    co_test_inner2: step 2!\n");
    suspend
    printf("    co_test_inner2: step 3!\n");
    suspend
    printf("    co_test_inner2: step 4!\n");
    result = 44;
    co_end
  }
};

class co_test: public coroutine
{
protected:
    
  co_test_inner1 test1;
  co_test_inner2 test2;
    
  virtual void main()
  {
    printf("  start/resume co_test\n");
    co_begin
    printf("  co_test: step 1!\n");
    suspend
    printf("  co_test: step 2!\n");
    await(test1)
    printf("test1 result = %d", test1.result);
    printf("  co_test: step 3!\n");
    suspend
    printf("  co_test: step 4!\n");
    await(test2)
    printf("test2 result = %d", test2.result);
    printf("  co_test: step 5!\n");
    co_end
  }
  
public:
    
};

int main() {
  co_test test;
  co_executor executor(&test);

  printf("before start\n");  
  executor.start();
  printf("main: step 1\n");
  executor.resume();
  printf("main: step 2, call read_completed()\n");
  read_completed();
  printf("main: step 3\n");
  executor.resume();
  printf("main: step 4\n");
  executor.resume();
  printf("main: step 5\n");
  
  return 0;
}