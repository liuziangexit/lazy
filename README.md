# lazy
[![Build Status](https://github.com/liuziangexit/lazy/workflows/ccpp/badge.svg)](https://github.com/liuziangexit/lazy/actions)
[![Build Status](https://travis-ci.com/liuziangexit/lazy.svg?branch=master)](https://travis-ci.com/liuziangexit/lazy)
<br>
lazy initialization implementation written in C++17.
<br>
懒初始化的C++17实现。
<h2>Features / 功能</h2>
-Lazy initialize / 懒初始化
<br>
-Thread-safe and Exception-safe / 线程安全和异常安全
<br>
-Support passing constructor arguments(Forwarding Reference) / 支持传递构造函数参数（完美转发）
<br>
-Efficient double check pattern / 高效的双检模式
<h2>Usage / 使用</h2>

```
#include "lazy.h"

struct big_class {
	big_class(const char* copy_me, unsigned int size) {
		//for demonstration purposes only, no out-of-range check, dont write this
		//仅用于演示，未进行越界检查，不要这样写
		for (unsigned int i = 0; i < size; i++)
			this->pretend_large_objects[i] = copy_me[i];
		this->pretend_large_objects[size] = 0;
	}

	char pretend_large_objects[8192];
};

int main() {
	//specify the type of the object which will be lazy initialize, and pass the constructor arguments(can be empty)
	//指定懒初始化对象的类型，并传入用于其构造函数的参数(可以没有参数)
	auto lazily_big_object = liuziangexit_lazy::make_lazy<big_class>((const char*)"naive", 5);

	//will be false
	//会是false
	bool is_created = lazily_big_object.is_instance_created();

	//the actual construction of that object happens here
	//实际的对象初始化发生在此处
	auto& big_object = lazily_big_object.get_instance();

	//will be true
	//会是true
	is_created = lazily_big_object.is_instance_created();

	auto& same_big_object = lazily_big_object.get_instance();
	//will be true, they point to the same object which stored in lazily_big_object
	//会是true，它们指向储存于lazily_big_object中的同一对象
	bool is_same = &same_big_object == &big_object;
}
//lazily_big_object and its managed objects will be destruct while leaving the scope
//离开作用域时，lazily_big_object及其管理之对象会被析构

```

<h2>Thread-safety / 线程安全</h2>
The following member function of class lazy is thread-safe:
<br>
- get_instance
<br>
- is_instance_created
<br><br>
类lazy的以下成员函数是线程安全的：
<br>
- get_instance
<br>
- is_instance_created
