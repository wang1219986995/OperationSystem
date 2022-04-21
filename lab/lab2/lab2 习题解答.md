# Part 1: Physical Page Management



#### Exercise 1.

> 在文件 kern/pmap.c中，你需要实现以下函数：
>
> `boot_alloc()`
>
> `mem_init()` (only up to the call to `check_page_free_list(1)`)
>
> `page_init()`
>
> `page_alloc()`
>
> `page_free()`
>
> `check_page_free_list()` 和 	`check_page_alloc()` 测试你的物理页分配器，你应该启动JOS并查看`check_page_alloc()` 会报成功。你会发现增加你自己的 assert() 判断你的假设是非常有帮助的。



