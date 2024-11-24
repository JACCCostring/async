#include <iostream>

#include "async.hpp"

struct MyS
{
    float x, y;

    MyS() : x{0.0f}, y{0.0f} {}
    MyS(float xs, float ys) : x{xs}, y{ys} {}

    bool operator!=(const MyS &r)
    {
        return x != r.x and y != r.y;
    }
};

template <>
class async::AsyncTask<MyS>::pimpl_type
{
public:
    pimpl_type() : _value{0.0f, 0.0f} {}

    MyS default_value() { return MyS{0.0f, 0.0f}; }

private:
    MyS _value;
};

class TestAsync : public async::AsyncTask<MyS>
{
public:
    TestAsync() = default;
};

class ReimplementAsync
{
public:
    ReimplementAsync() = default;

    void write(const MyS &content, std::size_t time = 1)
    {
        _async.set_sender([&content, time] -> MyS
                          { 
                            MyS temp;

                            std::this_thread::sleep_for(std::chrono::seconds(time));
                            
                            temp.x = content.x;
                            temp.y = content.y;

                            return temp; });
    }

    void read(async::AsyncTask<MyS>::Receiver_callback callback)
    {
        _async.on_receive([&](const MyS &content)
                          { callback(content); });
    }

    void start() { _async.run(); }

private:
    TestAsync _async;
};

int main(int argc, char **argv)
{
    ReimplementAsync temp;

    temp.write(MyS{59.31, -71.99}, 1);
    temp.write(MyS{60.31, -72.99}, 1);
    temp.write(MyS{61.31, -73.99}, 1);
    temp.write(MyS{62.31, -74.99}, 1);
    temp.write(MyS{63.31, -75.99}, 1);
    temp.write(MyS{64.31, -76.99}, 1);
    temp.write(MyS{65.31, -77.99}, 3);
    temp.write(MyS{66.31, -78.99}, 6);

    temp.read([](const MyS &s)
              { std::cout << s.x << " , " << s.y << std::endl; });

    /*while (true)
    {

        std::this_thread::sleep_for(std::chrono::seconds(1));

        temp.write(MyS{55.12, 89.06});

        std::this_thread::sleep_for(std::chrono::seconds(1));

        temp.write(MyS{56.12, 90.06});

        std::this_thread::sleep_for(std::chrono::seconds(1));

        temp.write(MyS{57.12, 91.06});

        // temp.read([](const MyS &s)
        //           { std::cout << s.x << " , " << s.y << std::endl; });

        std::cout << "back to main" << std::endl;

        temp.start();
    }*/

    std::cout << "back to main" << std::endl;

    temp.start();

    return 0;
}