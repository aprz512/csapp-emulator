csapp 模拟器实验

## 注意事项
编译时新建一个 build 目录，在build里面执行cmake，避免生成的临时文件污染仓库。

## 调试
在 .vscode 目录下添加 launch.json：
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "(gdb) Launch",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/linker",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}/build",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ]
        }
    ]
}
```
使用 cmake 生成好文件之后，在对应程序的入口源文件按下 F5 启动调试即可。

## 第一阶段
### 目标
完成一个简单的模拟器，写一个模拟器将 add/add.c 的汇编指令执行一遍。
具体的汇编指令在 add/add.txt 里面。 main 函数我们只取其中4条指令，add函数我们取全部指令。
模拟器将全部指令执行完成后，对比执行前后的寄存器与内存地址的值，一致则说明模拟器运行OK。
寄存器与内存地址的状态可以参考 add/check.txt。