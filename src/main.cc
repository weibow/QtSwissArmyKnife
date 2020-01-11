/*
 * Copyright (C) 2018-2019 wuuhii. All rights reserved.
 *
 * The file is encoding with utf-8 (with BOM). It is a part of QtSwissArmyKnife
 * project. The project is a open source project, you can get the source from:
 *     https://github.com/wuuhii/QtSwissArmyKnife
 *     https://gitee.com/wuuhii/QtSwissArmyKnife
 *
 * If you want to know more about the project, please join our QQ group(952218522).
 * In addition, the email address of the project author is wuuhii@outlook.com.
 */
#include "SAKApplication.hh"
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

bool cmp(int a, int b)
{
    return a < b;
}

int main(int argc, char *argv[])
{
    vector<int> myvec{3, 2, 5, 7, 3, 2};
    vector<int> lbvec(myvec);
    sort(myvec.begin(), myvec.end(), cmp);
    cout << "Predicate function:" << endl;
    for (int it : myvec)
        cout << it << ' ';
    cout << endl;

    sort(lbvec.begin(), lbvec.end(), [](int a, int b) -> bool {return a < b;});
    cout << "lambda expression:" << endl;
    for (int it : lbvec)
        cout << it << ' ';
    cout << "hello world" << endl;
    int b = 123;
    auto f = [b]{cout << b << endl;};
    b = 323;
    f();
    auto s = [&b]{cout << b << endl;};
    //b = 324;
    s();
   int m = [](int x) { return [](int y) { return y * 2; }(x)+6; }(5);
   cout << "m=" << m << endl;
    SAKApplication a(argc, argv);
    return a.exec();
}
