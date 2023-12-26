/* The MIT License

   Copyright (c) 2008, 2009, 2011 Attractive Chaos <attractor@live.co.uk>

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

/* Last Modified: 05MAR2012 */

// 如果标识符 AC_KSEQ_H 未定义（#ifndef 表示 "if not defined"），则执行后面的代码，否则跳过。这个标识符通常是头文件的宏定义，用于防止同一个头文件被多次包含。
#ifndef AC_KSEQ_H
// 在未定义的情况下，定义 AC_KSEQ_H 这个标识符，表示当前头文件已经被包含。
#define AC_KSEQ_H

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define KS_SEP_SPACE 0 // isspace(): \t, \n, \v, \f, \r  表示空白字符，包括制表符 \t、换行符 \n、纵向制表符 \v、换页符 \f 和回车符 \r
#define KS_SEP_TAB   1 // isspace() && !' '  表示空白字符且不是空格。
#define KS_SEP_LINE  2 // line separator: "\n" (Unix) or "\r\n" (Windows) 表示行分隔符，即换行符 \n（Unix）或回车换行符 \r\n
#define KS_SEP_MAX   2 // 表示行分隔符的最大值。

/* 
1. type_t 将在实际使用宏的地方被替换为实际的类型，从而使得结构体 __kstream_t 成为一个适用于不同类型输入流的通用结构。 
2. unsigned char *buf;：一个指向无符号字符的指针，用于存储读取的数据的缓冲区。
3. int begin, end, is_eof;：整数类型的成员，用于记录缓冲区的起始位置、结束位置和是否已经到达文件末尾。
4. type_t 是一个泛型类型的参数。这意味着在实际使用时，可以将不同的具体类型传递给 type_t，以满足不同的需求。泛型类型的目的是提供灵活性，使得这个结构体可以用于处理各种不同类型的输入流，比如文件指针、套接字描述符等。
在 C 语言中，泛型类型通常通过宏来实现，以实现在不同上下文中的通用性。在这里，type_t 只是一个占位符，实际使用时会通过宏展开为具体的类型。比如，可以将 type_t 替换为 FILE *，这样就表示 f 成员是一个文件指针。
或者将 type_t 替换为 int，这样就表示 f 是一个整数类型，可能代表套接字描述符。
*/
#define __KS_TYPE(type_t)						\
	typedef struct __kstream_t {				\
		unsigned char *buf;						\
		int begin, end, is_eof;					\
		type_t f;								\
	} kstream_t;

/*
1. ks_err(ks): 检查 kstream_t 结构体是否处于错误状态。如果 end 成员等于 -1，表示出现了错误，宏返回真（非零），否则返回假（零）。
2. ks_eof(ks): 检查 kstream_t 结构体是否处于文件末尾状态。结合 is_eof 和 begin、end 成员，如果已经到达文件末尾并且缓冲区中没有剩余可读数据，则返回真，否则返回假。
3. ks_rewind(ks): 重置 kstream_t 结构体的状态，将 is_eof、begin 和 end 成员都设为零，表示重新回到流的开头。
*/
#define ks_err(ks) ((ks)->end == -1)
#define ks_eof(ks) ((ks)->is_eof && (ks)->begin >= (ks)->end)
#define ks_rewind(ks) ((ks)->is_eof = (ks)->begin = (ks)->end = 0)

/*
1. static inline kstream_t *ks_init(type_t f)定义了一个静态内联函数 ks_init，该函数用于初始化 kstream_t 类型的结构体。
2. kstream_t *ks = (kstream_t*)calloc(1, sizeof(kstream_t))在函数内部定义了一个 kstream_t 类型的指针 ks，并使用 calloc 分配了一块内存来存储该结构体，并将其初始化为零。这里使用 calloc 而不是 malloc 是为了确保结构体中的所有字段都被初始化为零。
3. ks->f = f将结构体中的 f 字段（表示流对象）设置为传入的参数 f。
4. ks->buf = (unsigned char*)malloc(__bufsize) 为结构体中的缓冲区 buf 分配一块内存，大小为传入的参数 __bufsize。这里使用了 malloc 函数。
5. static inline void ks_destroy(kstream_t *ks)定义了一个静态内联函数 ks_destroy，用于销毁 kstream_t 类型的结构体。它接受一个指向流对象的指针作为参数。
6. free如果 ks 非空，就释放结构体中的缓冲区和结构体本身所占用的内存。这里使用了 free 函数。
*/
#define __KS_BASIC(type_t, __bufsize)								\
	static inline kstream_t *ks_init(type_t f)						\
	{																\
		kstream_t *ks = (kstream_t*)calloc(1, sizeof(kstream_t));	\
		ks->f = f;													\
		ks->buf = (unsigned char*)malloc(__bufsize);				\
		return ks;													\
	}																\
	static inline void ks_destroy(kstream_t *ks)					\
	{																\
		if (ks) {													\
			free(ks->buf);											\
			free(ks);												\
		}															\
	}

/*
1. static inline int ks_getc(kstream_t *ks)该函数名为 ks_getc，它接受一个指向 kstream_t 结构体的指针 ks 作为参数，并返回一个整数
2. if (ks_err(ks)) return -3 调用 ks_err(ks) 函数，该函数用于检查 kstream_t 结构体中的 end 成员是否为 -1，如果是，表示出现了错误，返回 -3。
3. if (ks->is_eof && ks->begin >= ks->end) return -1 检查 kstream_t 结构体中的 is_eof 是否为真（非零），并且 begin 大于等于 end。如果是，表示已经到达文件尾部，返回 -1。
4. if (ks->begin >= ks->end) 如果 begin 大于等于 end，说明缓冲区中没有字符可用。
5. ks->begin = 0 将 begin 重置为 0，表示缓冲区的下一个字符从头开始。
6. ks->end = __read(ks->f, ks->buf, __bufsize) 调用 __read 函数，该函数用于从输入流中读取数据，将数据存储到 ks->buf 缓冲区中，最多读取 __bufsize 个字节，并将实际读取的字节数赋值给 ks->end。
7. if (ks->end == 0) { ks->is_eof = 1; return -1;} 如果 end 为 0，表示已经到达文件尾部，将 is_eof 设为 1，返回 -1。
8. if (ks->end == -1) { ks->is_eof = 1; return -3;} 如果 end 为 -1，表示读取过程中发生了错误，将 is_eof 设为 1，返回 -3。
9. return (int)ks->buf[ks->begin++] 返回缓冲区中 begin 位置的字符，并将 begin 递增，指向下一个字符。
*/
#define __KS_GETC(__read, __bufsize)						\
	static inline int ks_getc(kstream_t *ks)				\
	{														\
		if (ks_err(ks)) return -3;							\
		if (ks->is_eof && ks->begin >= ks->end) return -1;	\
		if (ks->begin >= ks->end) {							\
			ks->begin = 0;									\
			ks->end = __read(ks->f, ks->buf, __bufsize);	\
			if (ks->end == 0) { ks->is_eof = 1; return -1;}	\
			if (ks->end == -1) { ks->is_eof = 1; return -3;}\
		}													\
		return (int)ks->buf[ks->begin++];					\
	}

/*
1. typedef struct __kstring_t {：这是定义一个结构体，结构体的名字是 __kstring_t。结构体中包含了两个成员变量：
size_t l：表示字符串的当前长度。
size_t m：表示字符串分配的内存空间大小。
char *s：表示字符串的字符数组。
} kstring_t;：结构体的定义结束，并使用 typedef 关键字将 __kstring_t 定义为 kstring_t 类型。这样就创建了一个新类型 kstring_t，它是一个结构体类型，包含了上述的成员变量
*/
#ifndef KSTRING_T
#define KSTRING_T kstring_t
typedef struct __kstring_t {
	size_t l, m;
	char *s;
} kstring_t;
#endif

/*
kroundup32(x) 宏的定义，它接受一个参数 x，并对 x 进行操作。
--(x) 先将 x 减一，这是因为我们想要找到不小于原值的最近的2的幂。
(x)|=(x)>>1，(x)|=(x)>>2，... 通过位运算将 x 的右侧的位设置为1，直到最高有效位（最左侧的1）。
++(x) 最后将 x 增加一，这样就得到了不小于原值的最近的2的幂。
这段代码的作用是定义了一个宏 kroundup32，用于将输入的32位整数 x 向上取到最近的2的幂。
*/
#ifndef kroundup32
#define kroundup32(x) (--(x), (x)|=(x)>>1, (x)|=(x)>>2, (x)|=(x)>>4, (x)|=(x)>>8, (x)|=(x)>>16, ++(x))
#endif

/*
1. define __KS_GETUNTIL(__read, __bufsize) 接受两个参数 __read 和 __bufsize。这里的 __read 通常是一个函数，用于从输入流中读取数据；__bufsize 是缓冲区的大小。
2. static int ks_getuntil2(kstream_t *ks, int delimiter, kstring_t *str, int *dret, int append)：这个函数用于从输入流中读取字符，直到指定的定界符或者流结束。参数解释如下：
ks: 输入流的状态信息。
delimiter: 定界符，可以是空格、制表符、换行符或用户指定的字符。
str: 保存读取的字符的字符串。
dret: 如果不为NULL，保存读取的定界符。
append: 如果为1，将读取的字符追加到字符串末尾；如果为0，覆盖原有字符串。
3. int gotany = 0 初始化一个标志，用于记录是否有字符被读取。
4. if (dret) *dret = 0 如果 dret 不为NULL，初始化为0，用于保存读取的定界符。
5. str->l = append? str->l : 0;：初始化字符串的长度。如果 append 为1，表示追加，则保持原有长度；否则，将长度设置为0。
6. for (;;) {：开始无限循环，直到遇到 break 跳出。
int i;：定义一个变量 i，用于保存定界符的位置。
if (ks_err(ks)) return -3;：如果输入流发生错误，返回-3。
if (ks->begin >= ks->end) {：如果缓冲区中没有数据了
if (!ks->is_eof) {：如果不是文件结束。
重置缓冲区起始位置 ks->begin 为0。
从输入流中读取数据到缓冲区。
如果读到了文件结束，设置 ks->is_eof 为1，并跳出循环。
else break;：如果是文件结束，跳出循环。
7. 接下来是根据不同的定界符类型，寻找定界符的位置 i
8. if (str->m - str->l < (size_t)(i - ks->begin + 1)) {：如果字符串的内存不足以容纳即将读取的字符。
将字符复制到字符串中。
更新字符串的长度。
更新缓冲区起始位置 ks->begin。
如果读取到了定界符，更新 dret。
如果读取到了文件结束且没有任何字符被读取，返回-1。
如果字符串为NULL，分配一个字节的内存。
如果是以回车符结尾，去掉回车符。
在字符串末尾添加终止符。
返回字符串的长度。
9. static inline int ks_getuntil(kstream_t *ks, int delimiter, kstring_t *str, int *dret)：这个函数是 ks_getuntil2 的封装，调用时将 append 参数设置为0。
*/
#define __KS_GETUNTIL(__read, __bufsize)								\
	static int ks_getuntil2(kstream_t *ks, int delimiter, kstring_t *str, int *dret, int append) \
	{																	\
		int gotany = 0;													\
		if (dret) *dret = 0;											\
		str->l = append? str->l : 0;									\
		for (;;) {														\
			int i;														\
			if (ks_err(ks)) return -3;									\
			if (ks->begin >= ks->end) {									\
				if (!ks->is_eof) {										\
					ks->begin = 0;										\
					ks->end = __read(ks->f, ks->buf, __bufsize);		\
					if (ks->end == 0) { ks->is_eof = 1; break; }		\
					if (ks->end == -1) { ks->is_eof = 1; return -3; }	\
				} else break;											\
			}															\
			if (delimiter == KS_SEP_LINE) {								\
				unsigned char *sep = memchr(ks->buf + ks->begin, '\n', ks->end - ks->begin); \
				i = sep != NULL ? sep - ks->buf : ks->end;				\
			} else if (delimiter > KS_SEP_MAX) {						\
				unsigned char *sep = memchr(ks->buf + ks->begin, delimiter, ks->end - ks->begin); \
				i = sep != NULL ? sep - ks->buf : ks->end;				\
			} else if (delimiter == KS_SEP_SPACE) {						\
				for (i = ks->begin; i < ks->end; ++i)					\
					if (isspace(ks->buf[i])) break;						\
			} else if (delimiter == KS_SEP_TAB) {						\
				for (i = ks->begin; i < ks->end; ++i)					\
					if (isspace(ks->buf[i]) && ks->buf[i] != ' ') break; \
			} else i = 0; /* never come to here! */						\
			if (str->m - str->l < (size_t)(i - ks->begin + 1)) {		\
				str->m = str->l + (i - ks->begin) + 1;					\
				kroundup32(str->m);										\
				str->s = (char*)realloc(str->s, str->m);				\
			}															\
			gotany = 1;													\
			memcpy(str->s + str->l, ks->buf + ks->begin, i - ks->begin); \
			str->l = str->l + (i - ks->begin);							\
			ks->begin = i + 1;											\
			if (i < ks->end) {											\
				if (dret) *dret = ks->buf[i];							\
				break;													\
			}															\
		}																\
		if (!gotany && ks_eof(ks)) return -1;							\
		if (str->s == 0) {												\
			str->m = 1;													\
			str->s = (char*)calloc(1, 1);								\
		} else if (delimiter == KS_SEP_LINE && str->l > 1 && str->s[str->l-1] == '\r') --str->l; \
		str->s[str->l] = '\0';											\
		return str->l;													\
	} \
	static inline int ks_getuntil(kstream_t *ks, int delimiter, kstring_t *str, int *dret) \
	{ return ks_getuntil2(ks, delimiter, str, dret, 0); }

#define KSTREAM_INIT(type_t, __read, __bufsize) \
	__KS_TYPE(type_t)							\
	__KS_BASIC(type_t, __bufsize)				\
	__KS_GETC(__read, __bufsize)				\
	__KS_GETUNTIL(__read, __bufsize)

#define kseq_rewind(ks) ((ks)->last_char = (ks)->f->is_eof = (ks)->f->begin = (ks)->f->end = 0)

#define __KSEQ_BASIC(SCOPE, type_t)										\
	SCOPE kseq_t *kseq_init(type_t fd)									\
	{																	\
		kseq_t *s = (kseq_t*)calloc(1, sizeof(kseq_t));					\
		s->f = ks_init(fd);												\
		return s;														\
	}																	\
	SCOPE void kseq_destroy(kseq_t *ks)									\
	{																	\
		if (!ks) return;												\
		free(ks->name.s); free(ks->comment.s); free(ks->seq.s);	free(ks->qual.s); \
		ks_destroy(ks->f);												\
		free(ks);														\
	}

/* Return value:
   >=0  length of the sequence (normal)
   -1   end-of-file
   -2   truncated quality string
   -3   error reading stream
 */
#define __KSEQ_READ(SCOPE) \
	SCOPE int kseq_read(kseq_t *seq) \
	{ \
		int c,r; \
		kstream_t *ks = seq->f; \
		if (seq->last_char == 0) { /* then jump to the next header line */ \
			while ((c = ks_getc(ks)) >= 0 && c != '>' && c != '@'); \
			if (c < 0) return c; /* end of file or error*/ \
			seq->last_char = c; \
		} /* else: the first header char has been read in the previous call */ \
		seq->comment.l = seq->seq.l = seq->qual.l = 0; /* reset all members */ \
		if ((r=ks_getuntil(ks, 0, &seq->name, &c)) < 0) return r;  /* normal exit: EOF or error */ \
		if (c != '\n') ks_getuntil(ks, KS_SEP_LINE, &seq->comment, 0); /* read FASTA/Q comment */ \
		if (seq->seq.s == 0) { /* we can do this in the loop below, but that is slower */ \
			seq->seq.m = 256; \
			seq->seq.s = (char*)malloc(seq->seq.m); \
		} \
		while ((c = ks_getc(ks)) >= 0 && c != '>' && c != '+' && c != '@') { \
			if (c == '\n') continue; /* skip empty lines */ \
			seq->seq.s[seq->seq.l++] = c; /* this is safe: we always have enough space for 1 char */ \
			ks_getuntil2(ks, KS_SEP_LINE, &seq->seq, 0, 1); /* read the rest of the line */ \
		} \
		if (c == '>' || c == '@') seq->last_char = c; /* the first header char has been read */	\
		if (seq->seq.l + 1 >= seq->seq.m) { /* seq->seq.s[seq->seq.l] below may be out of boundary */ \
			seq->seq.m = seq->seq.l + 2; \
			kroundup32(seq->seq.m); /* rounded to the next closest 2^k */ \
			seq->seq.s = (char*)realloc(seq->seq.s, seq->seq.m); \
		} \
		seq->seq.s[seq->seq.l] = 0;	/* null terminated string */ \
		if (c != '+') return seq->seq.l; /* FASTA */ \
		if (seq->qual.m < seq->seq.m) {	/* allocate memory for qual in case insufficient */ \
			seq->qual.m = seq->seq.m; \
			seq->qual.s = (char*)realloc(seq->qual.s, seq->qual.m); \
		} \
		while ((c = ks_getc(ks)) >= 0 && c != '\n'); /* skip the rest of '+' line */ \
		if (c == -1) return -2; /* error: no quality string */ \
		while ((c = ks_getuntil2(ks, KS_SEP_LINE, &seq->qual, 0, 1) >= 0 && seq->qual.l < seq->seq.l)); \
		if (c == -3) return -3; /* stream error */ \
		seq->last_char = 0;	/* we have not come to the next header line */ \
		if (seq->seq.l != seq->qual.l) return -2; /* error: qual string is of a different length */ \
		return seq->seq.l; \
	}

#define __KSEQ_TYPE(type_t)						\
	typedef struct {							\
		kstring_t name, comment, seq, qual;		\
		int last_char;							\
		kstream_t *f;							\
	} kseq_t;

#define KSEQ_INIT2(SCOPE, type_t, __read)		\
	KSTREAM_INIT(type_t, __read, 16384)			\
	__KSEQ_TYPE(type_t)							\
	__KSEQ_BASIC(SCOPE, type_t)					\
	__KSEQ_READ(SCOPE)

#define KSEQ_INIT(type_t, __read) KSEQ_INIT2(static, type_t, __read)

#define KSEQ_DECLARE(type_t) \
	__KS_TYPE(type_t) \
	__KSEQ_TYPE(type_t) \
	extern kseq_t *kseq_init(type_t fd); \
	void kseq_destroy(kseq_t *ks); \
	int kseq_read(kseq_t *seq);

#endif
