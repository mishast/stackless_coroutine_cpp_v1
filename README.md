# Stackless coroutine implementation

This is stackless coroutine implementation. It uses Duff's device.

It support nested coroutines.

State of coroutine stored in object.

Example:

```cpp
class CheckUserExists: public coroutine
{
public:
  std::string userName;

  bool        found;

  bool        error;
  std::string errorText;

protected:
  virtual void main()
  {
    co_begin

    std::string url = "/users/check/" + userName;

    result = false;
    error  = false;

    auto networkManager = NetworkManager::instance();

    networkManager->makeRequest(
      RequestMethod::GET,
      url,
      /* success */
      [this](std::string& response)
      {
        try
        {
          auto jresponse = nlohmann:json::parse(reponse);

          jresponse.at("found").get_to(found);
        }
        catch (...)
        {
          error = true;
          errorText = "Invalid response format";
        }
          
        executor->resume();
      },
      /* on error */
      [this](std::string& errorText)
      {
        error = true;
        errorText = error;
        executor->resume();
      }
    );

    suspend

    co_end
  }	
}


class co_main: public coroutine
{
public:
  std::string userName;
  std::string password;

  bool        error;
  std::string errorText;

protected:
    
  CheckUserExists checkUserExists;
  CreateUser      createUser;
  Login           login;
    
  virtual void main()
  {
    co_begin

    error = false;
    errorText = "";
    
    chekUserExists.userName = userName;
    await(checkUserExists);

    if (checkUserExists.error)
    {
      error = true;
      errorText = checkUserExists.errorText;
      co_return;
    }

    bool userExists = checkUserExists.found;
    
    if (!userExists)
    {
      createUser.userName = userName;
      createUser.password = password;
      await(createUser);

      if (createUser.error)
      {
        error = true;
        errorText = createUser.errorText;
        co_return
      }
    }

    login.userName = userName;
    password.password = password;
    await(login)

    if (login.error)
    {
      error = true;
      errorText = login.errorText;
      co_return   	
    }
    
    co_end
  }
};
```