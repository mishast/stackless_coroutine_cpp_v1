# Stackless coroutine implementation

This is stackless coroutine implementation. It uses Duff's device.

It support nested coroutines.

State of coroutine stored in object.

Example:

```
class co_main: public coroutine
{
protected:
    
  CheckUserExists checkUserExists;
  CreateUser      createUser;
  Login           login;
    
  virtual void main()
  {
    co_begin
    
    chekUserExists.userName = userName;
    await(checkUserExists);
    bool userExists = checkUserExists.result;
    
    if (!userExists)
    {
      createUser.userName = userName;
      createUser.password = password;
      await(createUser);

      if (!createUser.result)
      {
        result = false;
        co_return
      }
    }

    login.userName = userName;
    password.password = password;
    await(login)

    if (!login.result)
    {
      result = false;
      co_return   	
    }
    
    co_end
  }
  
public:
  std::string userName;
  std::string password;
  bool result;
};
```