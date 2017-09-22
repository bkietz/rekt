#include <string>

class move_only
{
public:
  move_only(move_only const &) = delete;
  move_only &operator=(move_only const &) = delete;

  move_only(std::string n)
      : n_(std::move(n))
  {
  }

  move_only(move_only&& x)
      : n_(std::move(x.n_))
  {
    x.n_ = "<moved>";
  }

  move_only &operator=(move_only&& x)
  {
    n_ = std::move(x.n_);
    x.n_ = "<moved>";
    return *this;
  }

  std::string const &name() const
  {
    return n_;
  }

private:
  std::string n_;
};
