#include <iostream>
#include "TextSimilarity.hpp"

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        cout << "Usage: 文本相似度<<"<< "路径名: " "<file1> " "<file2>" ">>";
        return 1;
    }
    Textlikey ts(argv[1]);
    cout << "cosine: "<<ts.getText_Similarity(argv[2], argv[3]);

    //测试用例
    //testWordFreq();
    //testOneHot();
    //TestWordLike();   
    return 0;
}

