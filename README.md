# objc4
## objc4源码阅读的仓库

官方源码地址： https://github.com/apple-oss-distributions/objc4 <br/>
解决问题的参考地址：https://juejin.cn/post/7049995719112917029 <br/>

**1、其实主要就是：**<font color=red> 添加lostFile到xcode中当做系统的头文件路径， 然后设置Search Paths中的 header search paths 设置$(SRCROOT)/lostFiles</font>

**2、其次就是讲有关的路径创建在对应的文件夹瞎买**