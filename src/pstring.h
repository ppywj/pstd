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
		 * ����ָ���ַ��ָ��ַ���
		 * @param v:���ڴ����ַ�����
		 * @param s:���ָ���ַ���
		 * @param delimiter:�ָ��ַ�
		 * @param maxSegments:����Ӵ��� Ĭ��Ϊ��0x7fffffff Ҳ����ʮ���Ƶ�2,147,483,647
		 * @return �ָ��õ����Ӵ�����
		 * @note v���Ȼᱻ���
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
				//��left��ʼ�����ҵ��ָ����λ��
				std::string::size_type right = s.find(delimiter, left);
				//��leftһֱ���ַ���ĩβ��û�зָ��
				if (right == std::string::npos) {
					break;
				}
				//�ҵ��˷ָ��ַ����ָ�֮������������Ӧ������λ��
				v.push_back(s.substr(left, right - left));
				//����������ʼλ��
				left = right + 1;
			}
			v.push_back(s.substr(left));
			return i;
		}

		/**
		 * ����ַ���ǰ��հ��ַ�
		 * @param s:Դ�ַ���
		 * @return ǰ����տհ��ַ����ַ���
		 */
		std::string trim(const std::string& s) {
			static const char* whiteSpace = " \t\r\n";
			//�մ�
			if (s.empty())
				return s;
			//�ҵ���һ����Ϊ�������ֿհ��ַ����ַ�������
			std::string::size_type b = s.find_first_not_of(whiteSpace);
			if (b == std::string::npos) // No non-spaces
				return "";
			//�ҵ����һ����Ϊ�հ��ַ����ַ�������
			std::string::size_type e = s.find_last_not_of(whiteSpace);
			//���ؽ�ȡ֮����ַ���
			return std::string(s, b, e - b + 1);
		}

		/**
		 * ����ַ���ǰ��հ��ַ�
		 * @param s:Դ�ַ���
		 * @return ǰ����տհ��ַ����ַ���
		 */
		std::string vform(const char* format, va_list args) {
			// ��һ�ε���vsnprintf��ȡ���軺������С
			va_list args_copy;
			va_copy(args_copy, args); // ����va_list�������һ�ε���Ӱ��ڶ���
			int needed_size = std::vsnprintf(nullptr, 0, format, args_copy);
			va_end(args_copy); // �ͷŵ�һ�ε��õ�args_copy
			// �����㹻��Ļ�����
			if (needed_size <= 0) {
				return std::string();
			}
			std::string result(needed_size + 1, '\0'); // +1��Ϊ�����ɽ�����'\0'
			// �ڶ��ε���vsnprintf����ʵ�ʵĸ�ʽ����ƴ��
			va_copy(args_copy, args); // �ٴθ���va_list
			std::vsnprintf(&result[0], needed_size + 1, format, args_copy);
			va_end(args_copy); // �ͷŵڶ��ε��õ�args_copy
			// ɾ������Ľ�����'\0'
			result.resize(needed_size);
			return result;
		}

		/**
		 * ת��Ϊ��д
		 * @param s:Դ�ַ���
		 * @return ת��Ϊ��д֮����ַ���
		 * @note Դ�ַ����ᱻ�޸�
		 */
		void to_upper(std::string& str)
		{
			std::transform(str.begin(), str.end(), str.begin(), ::toupper);
		}
		/**
		 * ת��Ϊ��д
		 * @param s:Դ�ַ���
		 * @return ת��Ϊ��д֮����ַ���
		 * @note Դ�ַ������ᱻ�޸�
		 */
		std::string to_upper_copy(const std::string& str)
		{
			std::string tmp = str;
			std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
			return tmp;
		}

		/**
	 * ת��ΪСд
	 * @param s:Դ�ַ���
	 * @return ת��ΪСд֮����ַ���
	 * @note Դ�ַ����ᱻ�޸�
	 */
		void to_lower(std::string& str)
		{
			std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		}

		/**
		 * ת��ΪСд
		 * @param s:Դ�ַ���
		 * @return ת��ΪСд֮����ַ���
		 * @note Դ�ַ������ᱻ�޸�
		 */
		std::string to_lower_copy(const std::string& str)
		{
			std::string tmp = str;
			std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
			return tmp;
		}

		/**
		 * ���Դ�Сд���ַ����Ƚ�
		 * @param s1,s2:����Ҫ�Ƚϵ��ַ���
		 * @return ���Դ�Сд֮���Ƿ����
		 */
		bool stringEqualIgnoreCase(const std::string& s1, const std::string& s2) {
			return s1.length() == s2.length() &&
				std::equal(s1.begin(), s1.end(), s2.begin(),
					[](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); });
		}

		/**
		 * ����kmp�㷨���ִ�����
		 * @param str:
		 * @param sonStr:
		 * @return ���Դ�Сд֮���Ƿ����
		 */

		int searchSonStr(const std::string& str, const std::string& sonStr)
		{
			std::vector<int>next;
			next.resize(str.length());
			next[0] = -1;
			signed int i = -1;//���ǰ׺��βָ��
			signed int j = 0;//����׺��βָ��
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
					//��i���ݵ�next[i]
					i = next[i];
				}
			}
			i = 0;
			j = 0;//�ֱ���str ��sonStr��ָ��
			while ((i < str.length()) && (j < (int)sonStr.length()))//ע�⣻length()���ص���size_t���� ��size_t��unsigned long long -11תΪsize_t����һ���ܴ����
			{
				if (j == -1 || sonStr[j] == str[i])
				{
					i++;
					j++;
				}
				else//i���� ��j�ƶ���next[j] 
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
		* �Ƿ����ĳ���Ӵ�
		* @param str:Դ�ַ���
		* @param sonStr:�Ӵ�
		* @return:��������true��֮����false
		*/
		bool contanins(const std::string& str, const std::string& sonStr)
		{
			return searchSonStr(str, sonStr) == -1;
		}

		/**
		* �Ƿ���ĳ���Ӵ���ͷ
		* @param str:Դ�ַ���
		* @param sonStr:�Ӵ�
		* @param toOffset:��Դ�ַ������ĸ�λ�ÿ�ʼ����,Ĭ��Ϊ0
		* @return:�����ָ��������ʼ��sonStr��ͷ�򷵻�true��֮����false
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
		* �Ƿ���ĳ���Ӵ���β
		* @param str:Դ�ַ���
		* @param sonStr:�Ӵ�
		* @return:���str��sonStr��β�򷵻�true��֮����false
		*/
		bool endWith(const std::string& str, const std::string& sonStr)
		{
			return startWith(str, sonStr, str.length() - sonStr.length());
		}
		/**
		* �滻�ַ��������޸�Դ�ַ���
		* @param str:�ַ���
		* @param oldChar:Ŀ���ַ�
		* @param newChar:�滻�ɵ��ַ�
		* @return :�滻֮����ַ���
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
		* �滻�ַ������޸�Դ�ַ���
		* @param str:�ַ���
		* @param oldChar:Ŀ���ַ�
		* @param newChar:�滻�ɵ��ַ�
		*/
		void replaceChar(std::string& str, char oldChar, char newChar) {
			for (char& c : str) {
				if (c == oldChar) {
					c = newChar;
				}
			}
		}

		/**
		* ������(����,С��,��������)תΪ�ַ���
		* @param i:Ҫת���Ĳ���
		* @return ת������ַ�������������Ͳ����������߸�������ô���ؿմ�
		*/
        template<typename T>
		std::string transFromNum(T i)
		{
			if (std::is_arithmetic<T>::value)//�������߸�����
			{
				return std::to_string(i);
			}
			else
				return "";
		}
	}
}