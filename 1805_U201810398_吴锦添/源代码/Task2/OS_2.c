#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include<linux/kfifo.h>
#include<linux/wait.h>
#include<linux/sched.h>
#include<linux/slab.h>
#include<linux/uaccess.h>

MODULE_LICENSE("GPL");

#define    Exp2_MAJOR     233
#define    DEVICE_NAME     "Exp2Module"
#define BUFFER_SIZE 64

DEFINE_KFIFO(FIFO_BUFFER,char,BUFFER_SIZE);//定义并初始化一个fifo，定义的时候调用的 DECLARE_KFIFO  ，所以也是静态分配 buf 。

unsigned int actual_read;  //定义实际从缓冲区读取的数据
unsigned int actual_write; //定义实际向缓冲区输入的数据

wait_queue_head_t read_queue;  //定义读队列
wait_queue_head_t write_queue; //定义写队列

static ssize_t FIFODev_Read(struct file *file, char __user * buf, size_t count, loff_t *ppos)  //利用kfifo结构编写读取模块
{
    if(kfifo_is_empty(&FIFO_BUFFER))   //如果缓冲区内没有数据 在判断阻塞与否后唤醒写操作
    {
        if(file->f_flags&O_NONBLOCK)  //f_flags是文件标志，用在文件本身已存在，指定打开该文件的方式
            return -EAGAIN;           //O_NONBLOCK判断是否是非堵塞式的操作 
            //EAGAIN表示如果你连续做read操作而没有数据可读，此时程序不会阻塞起来等待数据准备就绪返回，read函数会返回一个错误EAGAIN
            //，提示你的应用程序现在没有数据可读请稍后再试。
        wait_event_interruptible(read_queue,!kfifo_is_empty(&FIFO_BUFFER));  //直到缓冲区不空 唤醒read_queue队列
    } 
    kfifo_to_user(&FIFO_BUFFER,buf,count,&actual_read);  //进入这里就表示缓冲区不空了 可以从中读取数据 并且将数据存在actual_read中
    if(!kfifo_is_full(&FIFO_BUFFER))  //如果缓冲区没有满 表示可以继续写
    {
        wake_up_interruptible(&write_queue);   //唤醒写队列 即继续写
    }
    return actual_read;   //返回实际读取的数据
}

static ssize_t FIFODev_Write(struct file *file,const char __user * buf, size_t count, loff_t *ppos)  //写函数
{
    if(kfifo_is_full(&FIFO_BUFFER))  //当缓冲区满的时候 只能进行读操作 不能再写
    {
        if(file->f_flags&O_NONBLOCK)  //非阻塞状态
            return -EAGAIN;   //非阻塞
        wait_event_interruptible(write_queue,!kfifo_is_full(&FIFO_BUFFER));  //当缓冲区未满的时候 唤醒写队列
    }
    kfifo_from_user(&FIFO_BUFFER,buf,count,&actual_write);//进入这里就是缓冲区未满的情况
                            //向缓冲区中写入数据
    
    if(!kfifo_is_empty(&FIFO_BUFFER))  //如果缓冲区中有数据
        wake_up_interruptible(&read_queue); //唤醒读队列 可以从缓冲区中取出数据
    return actual_write;  //返回实际写入的数据
}


static struct file_operations FIFODev_flops = {   //将模块的写函数和读函数进行定义
    .owner  =   THIS_MODULE,
    .write  =   FIFODev_Write,
    .read   =   FIFODev_Read,
};

static int __init FIFODev_init(void){
    int ret;
    
    ret = register_chrdev(Exp2_MAJOR,DEVICE_NAME, &FIFODev_flops);   
    if (ret < 0) {
      printk(KERN_EMERG DEVICE_NAME " can't register major number.\n");
      return ret;
    }
    printk(KERN_EMERG DEVICE_NAME "Initialized.\n");
    init_waitqueue_head(&read_queue);   //初始化读队列
    init_waitqueue_head(&write_queue);  //初始化写队列
    return 0;
}

static void __exit FIFODev_exit(void){
    unregister_chrdev(Exp2_MAJOR, DEVICE_NAME);
    printk(KERN_EMERG DEVICE_NAME "Removed.\n");
}

module_init(FIFODev_init);
module_exit(FIFODev_exit);


//声明模块的作者（可选）
MODULE_AUTHOR("WuJintian");
//声明模块的描述（可选）  
MODULE_DESCRIPTION("This is OS_Exp2!/n");
//声明模块的别名（可选）  
MODULE_ALIAS("OS_Exp2");
