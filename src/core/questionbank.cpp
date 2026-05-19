#include "questionbank.h"
#include <QRandomGenerator>

static const QVector<Question>& bank() {
    static const QVector<Question> q = {
        {"C++中，虚函数的关键字是？",
         "virtual", "override", "abstract", "interface", 0},
        {"哪个关键字用于防止类被继承？",
         "final", "sealed", "static", "const", 0},
        {"多态是通过什么机制实现的？",
         "虚函数表(vtable)", "模板", "宏定义", "内联函数", 0},
        {"纯虚函数的声明方式是？",
         "= 0", "= null", "= delete", "= default", 0},
        {"C++中，派生类对象的构造顺序是？",
         "基类→成员→派生类", "派生类→基类→成员", "成员→基类→派生类", "基类→派生类→成员", 0},
        {"哪个访问控制符允许派生类访问基类成员？",
         "protected", "private", "public", "friend", 0},
        {"C++中，析构函数何时应声明为virtual？",
         "当类作为基类使用时", "所有情况", "仅在有多重继承时", "仅当有纯虚函数时", 0},
        {"override关键字的作用是？",
         "确保函数确实重写了基类虚函数", "提高运行效率", "允许函数重载", "隐藏基类函数", 0},
        {"dynamic_cast用于什么场景？",
         "运行时的安全向下转型", "编译时的类型转换", "基本类型转换", "函数指针转换", 0},
        {"C++中，抽象类是指？",
         "包含至少一个纯虚函数的类", "没有构造函数的类", "被final修饰的类", "只有公有成员的类", 0},
        {"RAII的核心思想是？",
         "资源获取即初始化，利用对象生命周期管理资源", "手动管理内存", "使用垃圾回收", "延迟初始化", 0},
        {"哪种继承方式下，基类的public成员在派生类中变为protected？",
         "protected继承", "public继承", "private继承", "virtual继承", 0},
        {"C++中，对象的成员初始化列表执行顺序取决于？",
         "成员在类中声明的顺序", "初始化列表中的顺序", "成员的字母顺序", "随机顺序", 0},
        {"关于虚基类，以下说法正确的是？",
         "用于解决菱形继承中的二义性", "使得基类成员变为私有", "禁止基类被直接实例化", "提高继承时的性能", 0},
        {"C++中，delete和delete[]的区别是？",
         "delete用于单个对象，delete[]用于数组", "没有区别", "delete[]用于单个对象", "delete用于释放类对象", 0},
        {"const成员函数中不能做什么？",
         "修改成员变量（除非mutable）", "访问成员变量", "调用其他const函数", "返回const引用", 0},
        {"智能指针unique_ptr的特点是？",
         "独占所有权，不可复制", "共享所有权", "不管理内存", "可被多个指针共同持有", 0},
        {"C++的多态分为哪两种？",
         "编译时多态和运行时多态", "静态绑定和动态绑定", "函数重载和模板", "以上都对", 3},
        {"友元函数可以访问类的哪些成员？",
         "所有成员（public, protected, private）", "仅public成员", "public和protected", "仅private成员", 0},
        {"C++中，移动语义的核心目的是？",
         "避免不必要的深拷贝，转移资源所有权", "代替引用传递", "实现多态", "简化语法", 0},
    };
    return q;
}

Question QuestionBank::drawRandom() {
    const auto& b = bank();
    return b[QRandomGenerator::global()->bounded(b.size())];
}

int QuestionBank::size() {
    return bank().size();
}
