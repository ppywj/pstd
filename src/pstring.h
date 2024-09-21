#pragma once
#include<string>
#include<vector>
#include<cstdarg>
#include<cctype>
#include<algorithm>
namespace pstd {
	namespace PStringUtils
	{

		/**
		 * 根据指定字符分割字符串
		 * @param v:用于储存字符串得
		 * @param s:待分割的字符串
		 * @param delimiter:分割字符
		 * @param maxSegments:最大子串数 默认为：0x7fffffff 也就是十进制的2,147,483,647
		 * @return 分割后得到的子串个数
		 * @note v首先会被清空
		 */
		 //log4cpp
		unsigned int split(std::vector<std::string>& v,
			const std::string& s, char delimiter,
			unsigned int maxSegments = INT_MAX)
		{
			v.clear();
			std::string::size_type left = 0;
			unsigned int i;
			for (i = 1; i < maxSegments; i++) {
				//从left开始往后找到分割符的位置
				std::string::size_type right = s.find(delimiter, left);
				//从left一直到字符串末尾都没有分割符
				if (right == std::string::npos) {
					break;
				}
				//找到了分割字符，分割之后存入迭代器对应的容器位置
				v.push_back(s.substr(left, right - left));
				//更新搜索起始位置
				left = right + 1;
			}
			v.push_back(s.substr(left));
			return i;
		}

		/**
		 * 清除字符串前后空白字符
		 * @param s:源字符串
		 * @return 前后清空空白字符的字符串
		 */
		std::string trim(const std::string& s) {
			static const char* whiteSpace = " \t\r\n";
			//空串
			if (s.empty())
				return s;
			//找到第一个不为上面四种空白字符的字符的索引
			std::string::size_type b = s.find_first_not_of(whiteSpace);
			if (b == std::string::npos) // No non-spaces
				return "";
			//找到最后一个不为空白字符的字符的索引
			std::string::size_type e = s.find_last_not_of(whiteSpace);
			//返回截取之后的字符串
			return std::string(s, b, e - b + 1);
		}

		/**
		 * 清除字符串前后空白字符
		 * @param s:源字符串
		 * @return 前后清空空白字符的字符串
		 */
		std::string vform(const char* format, va_list args) {
			// 第一次调用vsnprintf获取所需缓冲区大小
			va_list args_copy;
			va_copy(args_copy, args); // 复制va_list，避免第一次调用影响第二次
			int needed_size = std::vsnprintf(nullptr, 0, format, args_copy);
			va_end(args_copy); // 释放第一次调用的args_copy
			// 分配足够大的缓冲区
			if (needed_size <= 0) {
				return std::string();
			}
			std::string result(needed_size + 1, '\0'); // +1是为了容纳结束符'\0'
			// 第二次调用vsnprintf进行实际的格式化和拼接
			va_copy(args_copy, args); // 再次复制va_list
			std::vsnprintf(&result[0], needed_size + 1, format, args_copy);
			va_end(args_copy); // 释放第二次调用的args_copy
			// 删除多余的结束符'\0'
			result.resize(needed_size);
			return result;
		}

		/**
		 * 转化为大写
		 * @param s:源字符串
		 * @return 转化为大写之后的字符串
		 * @note 源字符串会被修改
		 */
		void to_upper(std::string& str)
		{
			std::transform(str.begin(), str.end(), str.begin(), ::toupper);
		}
		/**
		 * 转化为大写
		 * @param s:源字符串
		 * @return 转化为大写之后的字符串
		 * @note 源字符串不会被修改
		 */
		std::string to_upper_copy(const std::string& str)
		{
			std::string tmp = str;
			std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
			return tmp;
		}

		/**
	 * 转化为小写
	 * @param s:源字符串
	 * @return 转化为小写之后的字符串
	 * @note 源字符串会被修改
	 */
		void to_lower(std::string& str)
		{
			std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		}

		/**
		 * 转化为小写
		 * @param s:源字符串
		 * @return 转化为小写之后的字符串
		 * @note 源字符串不会被修改
		 */
		std::string to_lower_copy(const std::string& str)
		{
			std::string tmp = str;
			std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
			return tmp;
		}

		/**
		 * 忽略大小写的字符串比较
		 * @param s1,s2:两个要比较的字符串
		 * @return 忽略大小写之后是否相等
		 */
		bool stringEqualIgnoreCase(const std::string& s1, const std::string& s2) {
			return s1.length() == s2.length() &&
				std::equal(s1.begin(), s1.end(), s2.begin(),
					[](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); });
		}

		/**
		 * 基于kmp算法得字串查找
		 * @param str:
		 * @param sonStr:
		 * @return 忽略大小写之后是否相等
		 */

		int searchSonStr(const std::string& str, const std::string& sonStr)
		{
			std::vector<int>next;
			next.resize(str.length());
			next[0] = -1;
			signed int i = -1;//最大前缀的尾指针
			signed int j = 0;//最大后缀的尾指针
			while (j < str.length() - 1)
			{
				if (i == -1 || str[j] == str[i])
				{
					j++;
					i++;
					next[j] = i;
				}
				else
				{
					//将i回溯到next[i]
					i = next[i];
				}
			}
			i = 0;
			j = 0;//分别是str 和sonStr的指针
			while ((i < str.length()) && (j < (int)sonStr.length()))//注意；length()返回的是size_t类型 而size_t是unsigned long long -11转为size_t将是一个很大的数
			{
				if (j == -1 || sonStr[j] == str[i])
				{
					i++;
					j++;
				}
				else//i不动 将j移动到next[j] 
				{
					j = next[j];
				}

			}
			if (j >= sonStr.length())
			{
				return (i - sonStr.length());
			}
			else
				return -1;
		}

		/**
		* 是否包含某个子串
		* @param str:源字符串
		* @param sonStr:子串
		* @return:包含返回true反之返回false
		*/
		bool contanins(const std::string& str, const std::string& sonStr)
		{
			return searchSonStr(str, sonStr) == -1;
		}

		/**
		* 是否以某个子串开头
		* @param str:源字符串
		* @param sonStr:子串
		* @param toOffset:从源字符串的哪个位置开始查找,默认为0
		* @return:如果在指定索引开始以sonStr开头则返回true反之返回false
		*/
		bool startWith(const std::string& str, const std::string& sonStr, int toOffset = 0)
		{
			if (toOffset<0 || toOffset>sonStr.length() + toOffset > str.length())
			{
				return false;
			}
			for (int i = toOffset; i < sonStr.length(); ++i)
			{
				if (str[i] != sonStr[i - toOffset])
					return false;
			}
			return true;
		}


		/**
		* 是否以某个子串结尾
		* @param str:源字符串
		* @param sonStr:子串
		* @return:如果str以sonStr结尾则返回true反之返回false
		*/
		bool endWith(const std::string& str, const std::string& sonStr)
		{
			return startWith(str, sonStr, str.length() - sonStr.length());
		}
		/**
		* 替换字符，不会修改源字符串
		* @param str:字符串
		* @param oldChar:目标字符
		* @param newChar:替换成的字符
		* @return :替换之后的字符串
		*/
		std::string replaceChar_copy(const std::string& str, char oldChar, char newChar) {
			std::string result = str;
			for (char& c : result) {
				if (c == oldChar) {
					c = newChar;
				}
			}
			return result;
		}
		/**
		* 替换字符，会修改源字符串
		* @param str:字符串
		* @param oldChar:目标字符
		* @param newChar:替换成的字符
		*/
		void replaceChar(std::string& str, char oldChar, char newChar) {
			for (char& c : str) {
				if (c == oldChar) {
					c = newChar;
				}
			}
		}

		/**
		* 将数字(整数,小数,包括正负)转为字符串
		* @param i:要转换的参数
		* @return 转换后的字符串如果参数类型不是整数或者浮点数那么返回空串
		*/
        template<typename T>
		std::string transFromNum(T i)
		{
			if (std::is_arithmetic<T>::value)//整数或者浮点数
			{
				return std::to_string(i);
			}
			else
				return "";
		}
	}
}