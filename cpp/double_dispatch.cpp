// Example program
#include <iostream>
#include <string>

using namespace std;

class Base2;

class Base1 {
    public:
        virtual void op(const Base2& rhs) const = 0;
};

class A1 : public Base1 {
    void op(const Base2& rhs) const override;
};

class B1 : public Base1 {
    void op(const Base2& rhs) const override;
};

class Base2 {
    public:
    virtual void op(const A1& lhs) const = 0;
    virtual void op(const B1& lhs) const = 0;
};

void A1::op(const Base2& rhs) const { rhs.op(*this); }
void B1::op(const Base2& rhs) const { rhs.op(*this); }

class A2 : public Base2 {
    void op(const A1& lhs) const override {
        cout << "A1 op A2" << endl;
    }
    void op(const B1& lhs) const override {
        cout << "B1 op A2" << endl;
    }
};

class B2 : public Base2 {
    void op(const A1& lhs) const override {
        cout << "A1 op B2" << endl;
    }
    void op(const B1& lhs) const override {
        cout << "B1 op B2" << endl;
    }
};

// define virtual operator Base1 op Base2

void test(const Base1& b1, const Base2& b2) {
    b1.op(b2);
}

int main()
{
    test(A1(), A2());
    test(A1(), B2());
    test(B1(), A2());
    test(B1(), B2());
}
