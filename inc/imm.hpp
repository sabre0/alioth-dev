#ifndef __imm__
#define __imm__

#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>
#include "operatordef.hpp"
#include "methoddef.hpp"
#include "eproto.hpp"

namespace alioth {
using namespace llvm;

/**
 * @struct imm : 立即单元
 * @desc :
 *  立即单元是表达式的计算产物
 *  它可能是立即对象,也可能是立即寻址
 *  比如一个元素本身,就是一个立即寻址
 *  一个基础数据类型的加法运算中间结果就是一个立即对象
 *  立即对象直接存储数据,也直接存储数据类型
 *  立即寻址存储地址,存储元素原型
 * 
 *  满足Alioth语言规定的数据类型的对象，指针，共称  单元 unit
 *  对象，指针，引用，重载都能被元素直接寻址，共称   实例 instance
 *  对对象来说，地址是元素
 *  对指针来说，地址是元素
 *  对被引用管理的单元，地址是引用
 *  对被重载管理的单元，地址是重载
 */
struct imm;
using $imm = agent<imm>;
class Sengine;
struct imm : thing {

    public:
        enum immt {
            ele, // v 是元素,对于VAR和PTR存储了对象的地址Obj*, 对于REF和VAL存储了对象的指针的地址Obj**
            ins, // v 是实例,对于VAR和PTR存储了对象本身Obj, 对于REF和VAL存储了对象的指针Obj*
            fun, // v 是函数
            mem, // v 是成员运算符
        };

    private:
        /**
         * @member t : 立即单元类型
         * @desc :
         *  此标记标定此立即单元是否为立即对象或其他类型的立即单元
         */
        immt t;
        /**
         * @member v : 地址或值
         * @desc :
         *  若此单元为立即寻址,v保存元素,元素的本质就是对象的地址
         *  此值可以用于产生load或store指令
         *  若此单元是立即对象,v保存对象
         *  若此对象为函数,则v实际上是函数入口GV
         */
        Value* v;

        /**
         * @member p : 元素原型或数据类型
         * @desc :
         *  若单元为立即寻址或立即对象,则其中存储元素原型eproto
         *  若单元为实体,则其中存储类定义ClassDef
         *  若单元为立即函数,则其中存储函数原型MethodDef
         */
        anything p;
    
    public:

        /**
         * @member h : 宿主
         * @desc :
         *  可选的,如果此对象表示成员运算的结果,h指向成员运算的宿主,避免重复计算
         */
        $imm h;

    public:
        imm() = default;
        imm( immt T, Value* V, anything P, $imm H = nullptr );
        imm( const imm& ) = default;
        imm( imm&& ) = default;
        imm& operator=( const imm& ) = default;
        imm& operator=( imm&& ) = default;
        ~imm() = default;

        bool is( immt )const;
        immt is()const;

        /** 存入元素 */
        static $imm element( Value* addr, $eproto proto, $imm host = nullptr );

        /** 存入实例 */
        static $imm instance( Value* obj, $eproto proto );

        $eproto eproto()const;

        static $imm entity( Value* addr, $ClassDef def );
        
        static $imm function( Value* fp, $MethodDef prototype, $imm host = nullptr );
        $MethodDef prototype()const;

        static $imm member( Value* fp, $OperatorDef member, $imm host = nullptr );
        $OperatorDef member()const;

        Value* raw()const;
        
        /** 
         * 获取可以直接参与运算的Value*
         * 1. 对于立即寻址,执行load 
         * 2. 对于引用和右值,再执行load
         */
        Value* asunit( IRBuilder<>& builder, Sengine& sengine )const;

        /**
         * 获取可以执行store存储对象的Value*
         * 1. 对于立即对象,REF和VAL,直接返回v,其他返回空
         * 2. 对于立即寻址,REF和VAL,执行load后返回,其他直接返回
         */
        Value* asaddress( IRBuilder<>& builder, Sengine& sengine )const;
        bool hasaddress()const;

        /**
         * 获取可用于传参Value* 要确保请求类型和源类型完全一致
         * 1. 对立即对象
         *      若请求引用或重载，失败
         *      对其他情况，直接返回
         * 2. 对立即寻址
         *      若请求对象
         *          若复合数据类型，直接返回
         *          否则，执行load后返回
         *      若请求指针
         *          若不是指针数据类型，失败
         *          否则，执行load后返回
         *      若请求引用或重载
         *          直接返回
         */
        Value* asparameter( IRBuilder<>& builder, Sengine& sengine, etype e )const;

        Value* asfunction()const;

};

using imms = chainz<$imm>;
using bundles = map<string,$imm>;

}
#endif