#include <iostream>
#include <utility>
#include <vector>

using namespace std;

class Student{
 public:
  Student(int age, string name)
    : age_(age)
    , name_(std::move(name))
  {
    printf("age = %d , name = %s",age_,name_.c_str());
  }
 private:
  int age_;
  string name_;
};

int main() {

  vector<Student> data;

  data.push_back(Student(13,"小杨"));

  //data.push_back(13,"小杨");    不允许这样操作

  data.emplace_back(13,"小杨");

  data.emplace_back(Student(13,"小杨"));  //允许这样操作，但不建议
  return 0;
}
