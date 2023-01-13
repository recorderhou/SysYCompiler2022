# SysYCompiler2022

2022秋北京大学编译原理lab [https://pku-minic.github.io/online-doc/#/]

这个编译器支持：
1. 表达式运算（+/-/*/!/==/!=/>=/<=/>/<）
2. int/一维int数组型常量/变量声明、定义、运算
2. if/else分支
3. while循环
4. int/void型函数声明和调用，SysY库函数调用（参数为int/int一维数组）

这个编译器不支持
1. 涉及多维数组的一切（也就是没有写lv9.2的意思）
2. 支持1-5包含的一切内容

请参考lab文档使用这个编译器

生成Koopa IR：
To Generate Koopa IR:
```
build/compiler -koopa hello.c -o hello.koopa
```
生成RISCV汇编：
To Generate RISCV:
```
build/compiler -riscv hello.c -o hello.riscv
```