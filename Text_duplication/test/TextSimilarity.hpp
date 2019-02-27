#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <string>
#include <unordered_set>
#include <cppjieba/Jieba.hpp>
#include <iostream>
#include <algorithm>
#include <assert.h>

using namespace std;

const char* const DICT_PATH = "/jieba.dict.utf8";
const char* const HMM_PATH = "/hmm_model.utf8";
const char* const USER_DICT_PATH = "/user.dict.utf8";
const char* const IDF_PATH = "/idf.utf8";
const char* const STOP_WORD_PATH = "/stop_words.utf8";

class Textlikey
{
public:
	typedef std::unordered_map<std::string, int> word_num;
	//map其中有一个key 与value
    typedef std::unordered_set<std::string> word_set;
	Textlikey(std::string dict);
	double getText_Similarity(const char* file1, const char* file2);
	word_num getWordNumber(const char* file);

	void get_StopWord(const char* stopWordFile);
	vector<pair<string, int>> sortByValueReverse(word_num& wf);
    std::vector<double> getFactor(word_set& wset, word_num& wf);
	void selectAimWords(std::vector<std::pair<std::string, int>>& wfvec, word_set& wset);
	double coslike(std::vector<double> factor1, std::vector<double> factor2);

private:

	//void get_StopWord(const char* stopWordFile);
	//vector<pair<string, int>> sortByValueReverse(wordFreq& wf);
	//void selectAimWords(std::vector<std::pair<std::string, int>>& wfvec, wordSet& wset);
	//用来给词编码的wset用来存储词
    //std::vector<double> getOneHot(wordSet& wset, wordFreq& wf);
    //构建词频向量
	//double coslike(std::vector<double> factor1, std::vector<double> factor2);
    //余弦相似度公式：点积/模积	
	std::string DICT;
	std::string DICT_PATH;
	std::string HMM_PATH;
	std::string USER_DICT_PATH;
	std::string IDF_PATH;
	std::string STOP_WORD_PATH;
	cppjieba::Jieba _jieba;

	word_set _stopWordSet;
	int _maxWordNumber;
};


Textlikey::Textlikey(std::string dict)
    :DICT(dict),
     DICT_PATH(dict + "/jieba.dict.utf8"),
	 HMM_PATH(dict + "/hmm_model.utf8"),
	 USER_DICT_PATH(dict + "/user.dict.utf8"),
     IDF_PATH(dict + "/idf.utf8"),
	 STOP_WORD_PATH(dict + "/stop_words.utf8"),
     _jieba(DICT_PATH,
         HMM_PATH,
         USER_DICT_PATH,
         IDF_PATH,
         STOP_WORD_PATH),
        _maxWordNumber(10)
{
    get_StopWord(STOP_WORD_PATH.c_str());
}

//判断文本是否相似
double Textlikey::coslike(std::vector<double> factor1, std::vector<double> factor2)
{
    double factor_num1 = 0, factor_num2 = 0;
    double products = 0;
    assert(factor1.size() == factor2.size());
    for (int i = 0; i < factor1.size(); i++)
    {
        products += factor1[i] * factor2[i];
    }

    for (int i = 0; i < factor1.size(); i++)
    {
        factor_num1 += pow(factor1[i], 2);
    }
    factor_num1 = pow(factor_num1, 0.5);
    
    for (int i = 0; i < factor2.size(); i++)
    {
        factor_num2 += pow(factor2[i], 2);
    }
    factor_num2 = pow(factor_num2, 0.5);
    return products / (factor_num2 * factor_num1);
}
vector<double> Textlikey::getFactor(word_set& wset, word_num& wf)
{
    //遍历word_set中的每一个词
    vector<double> factor;
    for (const auto&e : wset)
    {
        if (wf.count(e))
            //factor就是value
            factor.push_back(wf[e]);
        else
            factor.push_back(0);
    }
    return factor;
}


//选择词频前几个词
void Textlikey::selectAimWords(vector<pair<string, int>>&wfvec, word_set& wset)
{
    int len = wfvec.size();
    int sz = (len > _maxWordNumber ? _maxWordNumber : len);
    for (int i = 0; i < sz ; ++i)
    {
        //pair<string, int>只要string
        wset.insert(wfvec[i].first);
    }
}

//停用词
void Textlikey::get_StopWord(const char* stopWordFile)
{
    ifstream fp(stopWordFile);
    if (!fp.is_open())
    {
        cout << "open file error" << endl;
        return;
    }

    string line;
    while (!fp.eof())
    {
        getline(fp, line);
        _stopWordSet.insert(line);
    }
    fp.close();
}

bool cmpReverse(pair<string, int> lp, pair<string, int>rp)
{
    //sort函数只支持序列容器,把map转成vector
    return lp.second > rp.second;
}

vector<pair<string, int>> Textlikey::sortByValueReverse(word_num& wf)
{
    //unordered_map
    vector<pair<string, int>> wfvector(wf.begin(), wf.end());
    sort(wfvector.begin(), wfvector.end(), cmpReverse);
    return wfvector;
}

Textlikey::word_num Textlikey::getWordNumber(const char* file)
{
    ifstream ifs(file);
    string len;
    word_num wf;
    if (!ifs.is_open())
    {
        cout << "ifstream open file error" << endl;
        return word_num();
    }
    
    while (!ifs.eof())
    {
        getline(ifs, len);
        vector<string> words;
        
        //对当前文本分词
        _jieba.Cut(len, words, true);
        
        //统计词频
        for (const auto& e : words)
        {
            //去掉停用词
            if (_stopWordSet.count(e) > 0)
                continue;
            else
            {
                //统计停用词
                if (wf.count(e) > 0)
                    wf[e]++;
                else
                    wf[e] = 1;
            }
        }
    }
    return wf;
}

double Textlikey::getText_Similarity(const char* file1, const char* file2)
{
    Textlikey::word_num wf = getWordNumber(file1);
    Textlikey::word_num wf1 = getWordNumber(file2);

    vector<pair<string, int>> wfvec = sortByValueReverse(wf);
    vector<pair<string, int>> wfvec1 = sortByValueReverse(wf1);

    cout << "wfvec:____________________________________________" << endl;
    for (int i = 0; i < 10; i++)
    {
        cout << wfvec[i].first << ": " << wfvec[i].second << endl; 
    }
    cout << "wfvec1:___________________________________________" << endl;
    for (int i = 0; i < 10; i++)
    {
        cout << wfvec1[i].first << ": " << wfvec1[i].second << endl; 
    }
    
    Textlikey::word_set wset;
    //用排好序的词频挑选前n个候选词
    selectAimWords(wfvec, wset);
    //用排好序的词频挑选前n个候选词
    selectAimWords(wfvec1, wset);
    cout << "add_wset:_________________________________________" << endl;
    for (const auto& e : wset)
    {
        cout << e << " " << endl;
    }
    cout << endl;

    //构建词频向量    
    vector<double> factor = getFactor(wset, wf);
    vector<double> factor1 = getFactor(wset, wf1);
    cout << "factor:------------------------------------------------------" << endl;
    for (const auto& v : factor)
    {
        cout << v << " ";
    }
    cout << endl;
    cout << "factor1:------------------------------------------------------" << endl;
    for (const auto& v : factor1)
    {
        cout << v << " ";
    }
    cout << endl;
	double cosine = coslike(factor, factor1);
    //cout << "cosine: " << cosine << endl;
    return cosine;
}



/*
void testWordFreq()
{
    Textlikey ts("./dict");
    Textlikey::word_num wf = ts.getWordNumber("pthread.txt");
    cout << "wf:-----------------------------------------------------------" << endl;
    for (const auto& w : wf)
    {
        cout << w.first << ":" << w.second <<endl;
    }
    cout << "wfvec:---------------------------------------------------------" << endl;
    vector<pair<string, int>> wfvec = ts.sortByValueReverse(wf);
    for (const auto& w : wfvec)
    {
        cout << w.first << ":" << w.second << endl;
    }
    
}
*/
/*
void testOneHot()
{
    Textlikey ts("./dict");
    Textlikey::word_num wf = ts.getWordNumber("pthread.txt");
    Textlikey::word_num wf2 = ts.getWordNumber("pthread1.txt");
    vector<pair<string, int>> wfvec = ts.sortByValueReverse(wf);
    vector<pair<string, int>> wfvec2 = ts.sortByValueReverse(wf2);
    //Textlikey::word_set wset;
    cout << "wfvec:------------------------------------------------------" << endl;
    for (int i = 0; i < 10; ++i)
    {
        cout << wfvec[i].first << ":" << wfvec[i].second << endl;
    }
    //用排好序的词频挑选前n个候选词
    ts.selectAimWords(wfvec, wset);
    //根据候选词构建词频向量
    vector<double> factor = ts.getFactor(wset, wf);
    cout << "wset:--------------------------------------------------------" << endl;
    for (const auto& e : wset)
    {
        cout << e << " " << endl;
    }
    cout << "factor:------------------------------------------------------" << endl;
    for (const auto& v : factor)
    {
        cout << v << " ";
    }
    cout << endl;
}
*/

/*
void TestWordLike()
{
    Textlikey ts("./dict");

    Textlikey::word_num wf = ts.getWordNumber("pthread.txt");
    Textlikey::word_num wf1 = ts.getWordNumber("pthread1.txt");

    vector<pair<string, int>> wfvec = ts.sortByValueReverse(wf);
    vector<pair<string, int>> wfvec1 = ts.sortByValueReverse(wf1);

    cout << "wfvec:____________________________________________" << endl;
    for (int i = 0; i < 10; i++)
    {
        cout << wfvec[i].first << ": " << wfvec[i].second << endl; 
    }
    cout << "wfvec1:___________________________________________" << endl;
    for (int i = 0; i < 10; i++)
    {
        cout << wfvec1[i].first << ": " << wfvec1[i].second << endl; 
    }
    
    Textlikey::word_set wset;
    //用排好序的词频挑选前n个候选词
    ts.selectAimWords(wfvec, wset);
    //用排好序的词频挑选前n个候选词
    ts.selectAimWords(wfvec1, wset);
    cout << "add_wset:_________________________________________" << endl;
    for (const auto& e : wset)
    {
        cout << e << " " << endl;
    }
    cout << endl;

    //构建词频向量    
    vector<double> factor = ts.getFactor(wset, wf);
    vector<double> factor1 = ts.getFactor(wset, wf1);
    cout << "factor:------------------------------------------------------" << endl;
    for (const auto& v : factor)
    {
        cout << v << " ";
    }
    cout << endl;
    cout << "factor1:------------------------------------------------------" << endl;
    for (const auto& v : factor1)
    {
        cout << v << " ";
    }
    cout << endl;
	double cosine = ts.coslike(factor, factor1);
    cout << "cosine: " << cosine << endl;
}

*/


#if 0
int main(int argc, char** argv) {
    testWordFreq();
    testOneHot();

    int main(int argc, char** argv)
    {
        if (argc != 4)
        {
            cout << "user: " << argv[0] << "<dict-dir> <file1> <file2>" << endl;
            return 1;
        }

        TextSimilarity ts(argv[1]);
        cout << ts.getTextSimilarity(argv[1], argv[2]);
    }
    
    
    
    
    /*    
cppjieba::Jieba jieba(DICT_PATH,
        HMM_PATH,
        USER_DICT_PATH,
        IDF_PATH,
        STOP_WORD_PATH);
  vector<string> words;
  vector<cppjieba::Word> jiebawords;
  
  
  string s;
  string result;
    
  
  cout << "[demo] Cut With HMM" << endl;
  jieba.Cut(s, words, true);
  cout << limonp::Join(words.begin(), words.end(), "/") << endl;
  
  s = "他来到了网易杭研大厦";
  cout << s << endl;
  cout << "[demo] Cut With HMM" << endl;
  jieba.Cut(s, words, true);
  cout << limonp::Join(words.begin(), words.end(), "/") << endl;

  cout << "[demo] Cut Without HMM " << endl;
  jieba.Cut(s, words, false);
  cout << limonp::Join(words.begin(), words.end(), "/") << endl;

  s = "我来到北京清华大学";
  cout << s << endl;
  cout << "[demo] CutAll" << endl;
  jieba.CutAll(s, words);
  cout << limonp::Join(words.begin(), words.end(), "/") << endl;
*/
  
  return 0;
}
#endif

