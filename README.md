# .Snake
C课设代码，用EasyX实现的简单贪吃蛇。

## 控制
WASD移动贪吃蛇
支持一帧内连续的操作

JK控制游戏速度，这会影响分数

## 食物
有多种食物，介绍略
部分食物有时间限制，请注意食物旁的绿条

## 分数
以从上次食物到下次食物的步数相关联的分数为基础，根据游戏速度和食物种类进行倍率计算

## 自定义
你可以按T键切换主题色
同时有一种食物可以切换主题色

你可以放置一张bg.jpg在游戏程序根目录下，文件将被加密并生成bg文件

当存在bg文件在游戏程序根目录下时，bg图片将以填充遮罩的方式作为游戏背景
当分数达到1w分时图片将会以bg.jpg的形式解密

你可以放置音乐文件在游戏程序根目录下，对文件名有什么要求我忘了，请参考源代码。
