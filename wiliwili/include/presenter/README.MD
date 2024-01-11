# wiliwili Presenter

调用B站接口的逻辑层封装

一般来说 fragment中的各个页面会继承Presenter下的各个页面请求逻辑封装类，在需要的时候(一般是构造函数里)调用requestData()
，开始请求数据。 当请求的数据完成后，各个页面中的回调函数被触发。

示例：

presenter/demo.hpp

```c++
class Demo {
public:
    virtual void onSuccess(const bilibili::DemoResult &result){}

    void requestData() {
        this->request(1, 24);
    }

    void request(int index = 1, int num = 24) {
        // 异步执行的网络请求
        async.run([this, index, num](const bilibili::DemoResult &result){
        // 异步执行结束后，调用：
            this->onSuccess(result);
        });
    }
};
```

fragment/page_demo.hpp

```c++
class PageDemo : public Demo, public brls::Box{
    PageDemo(){
        this->inflateFromXMLRes("xml/fragment/demo.xml");
        this->requestData();
    }
    
    void onSuccess(const bilibili::DemoResult &result){
        // 获取到数据，更新界面
        
        brls::Threading::sync([this, result](){
            // 注意：部分组件的更新需要在主线程
        });
    }
};
```