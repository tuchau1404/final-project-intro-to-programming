#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <bits/stdc++.h>



//variable
const int maxN=30000,rowNum=120,colNum=216,inf=1e9;
const int maxElement=rowNum*colNum;
//bfs variable
int distance[maxElement],trace[maxElement];
bool visit[maxElement];
std::vector<int> specialTable[maxElement];
std::vector <int> path;
//preprocess variable
std::vector<std::pair<std::string,int>> codeBuilding;

//function
void run();
bool checkBorder(int ci,int cj);
void map2AdjList(char srcFileName[],char dstFileName[],bool exportData);
void countRowCol(char fileName[]);
void exportImg(char fileName[]);

//bfs function
void bfs(int src,int dst);

//preprocess
std::tuple<bool,int,int> srcDst(char src[],char dst[]);
void checkCodeBuildings(char fileName[]);
int findOrderNode(char fileName[],int code);
int rowCol2Node(int indexRow, int indexCol);
std::pair<int,int> node2RowCol(int orderNode);
void listLocation(char fileName[]);
//show path of map
void showPath(char mapFile[],char pathFile[]);


int main()
{
//    countRowCol("map(origin).txt");
    run();


    return 0;

}

void run()
{
    bool checkOut=0;
    char src[100],dst[100];
    char conditionOut=' ';

    char mapLocation[]="map(origin).txt";
    //set up program
    listLocation("code_buildings.txt");
    map2AdjList(mapLocation,"map_adj_list.txt",false);

    //interface
    printf("Hi, welcome to our final project\n\n");
    printf("List of locations\n\n");

    for (auto building:codeBuilding)
        printf("%s\n", building.first.c_str());
    printf("\n");

    while (true)
    {
        if (conditionOut=='N')
            break;
        printf("Enter start location: ");
        scanf("%s",src);
        printf("Enter end location: ");
        scanf("%s",dst);
        printf("\n");

        auto [checkFlag,startCode,endCode]= srcDst(src,dst);
        if (!checkFlag)
        {
            printf("Opps, check start and end location name again!!!\n\n");
            continue;
        }
        int startPoint= findOrderNode(mapLocation,startCode);
        int endPoint= findOrderNode(mapLocation,endCode);

        //find the shortest path, save in path vector
        bfs(startPoint,endPoint);
        exportImg("the shortest path.pgm");
        showPath("VGU map 8k(resized).jpg","the shortest path.pgm");

        printf("Do you want to continue? Y/N\n");
        while (true)
        {
            scanf("%c",&conditionOut);
            if (conditionOut=='Y'||conditionOut=='N')
                break;
        }
    }
    printf("\nHave a nice day\n\n\n\n");
}





bool checkBorder(int ci,int cj)
{
    return (ci>=0&&ci<rowNum&&cj>=0&&cj<colNum);
}

void map2AdjList(char srcFileName[],char dstFileName[],bool exportData)
{

    int moveX[2]={1,0};
    int moveY[2]={0,1};
    int table[rowNum][colNum];
    FILE *fptrSrc,*fptrDst;
    fptrSrc= fopen(srcFileName,"r");
    fptrDst= fopen(dstFileName,"w");

    //save data
    for (int i =0; i<rowNum;i++)
        for (int j=0; j<colNum;j++)
            fscanf(fptrSrc,"%d",&table[i][j]);

    //convert data to adjacency list
    for (int i =0; i<rowNum;i++)
        for (int j=0;j<colNum;j++)
        {
            if (table[i][j]==0)
                continue;
            for (int k=0;k<2;k++)
            {
                int vi = i+moveY[k];
                int vj = j+moveX[k];
                if(!checkBorder(vi,vj))
                    continue;
                if(table[vi][vj]==0)
                    continue;
                int currentIndex= j+colNum*i;
                int adjacencyIndex= vj+colNum*vi;
                specialTable[currentIndex].push_back(adjacencyIndex);
                specialTable[adjacencyIndex].push_back(currentIndex);
                //printf("Hi: %d %d %d %d %d %d\n",i,j,vi,vj,currentIndex,adjacencyIndex);
            }
        }

    //save in file
    if(exportData)
    {
        for (int i=0;i<maxElement;i++)
        {
            if(specialTable[i].size()!=0)
            {
                fprintf(fptrDst,"%d ",specialTable[i].size());
                fprintf(fptrDst,"%d ", i);
                for (int j=0;j<specialTable[i].size();j++)
                    fprintf(fptrDst,"%d ",specialTable[i][j]);
                fprintf(fptrDst,"\n");
            }
        }
        //mark EOF
        fprintf(fptrDst,"%d",inf);
        fclose(fptrSrc);
        fclose(fptrDst);
    }
}

void countRowCol(char fileName[])
{
    FILE *fptr;
    int countRow =0;
    float avgCol,countChars=0;
    fptr= fopen(fileName,"r");
    char ch = fgetc(fptr);
    bool flag1=true;
    while (ch != EOF)
    {
        if((flag1==false&&ch==' ')||ch=='\n')
            flag1=true;


        if ((ch!=' ' && ch!='\n')&&flag1==true){
            countChars++;
            flag1=false;
        }
        if (ch=='\n'){
            countRow++;
        }
        ch = fgetc(fptr);

    }
    avgCol=countChars/countRow;
    printf("The count of row is:%i\n",countRow);
    printf("The count of column is:%f",avgCol);
    fclose(fptr);
}

void exportImg(char fileName[])
{
    FILE* fptr;
    fptr = fopen(fileName, "wb");

    // Writing Magic Number to the File
    fprintf(fptr, "P2\n");

    // Writing Width and Height
    fprintf(fptr, "%d %d\n", colNum, rowNum);

    // Writing the maximum gray value
    fprintf(fptr, "255\n");

    std::vector<int> sortedPath=path;
    sort(sortedPath.begin(),sortedPath.end());
    int currentIndexPath=0;

    for (int index =0;index<rowNum*colNum;index++)
    {
        if(index%colNum==0)
            fprintf(fptr, "\n");
        if(index==sortedPath[currentIndexPath]&&currentIndexPath!=sortedPath.size())
        {
            //0:black;
            fprintf(fptr, "%d ", 0);
            currentIndexPath++;
            continue;
        }
        //255: white
        fprintf(fptr, "%d ", 255);
    }

    path.clear();
    fclose(fptr);
}


void bfs (int src,int dst)
{
    //find trace and visit array
    for (int index=0;index<maxElement;index++)
    {
        distance[index]=0;
        trace[index]=-1;
        visit[index]=false;
    }
    std::queue <int> q;
    q.push(src);
    visit[src]=true;
    while(!q.empty())
    {
        int u = q.front();
        q.pop();
        for(int index =0;index<specialTable[u].size();index++)
        {
            int value =specialTable[u][index];
            if(!visit[value])
            {

                distance[value]=distance[u]+1;
                trace[value]=u;
                visit[value]= true;
                q.push(value);
            }
        }
    }
    //show the shortest path
    if(!visit[dst])
        printf("No path");
    else
    {

        for (int index = dst; index!=-1;index=trace[index])
            path.push_back(index);
        reverse(path.begin(),path.end());
//        printf("Path: ");
//        for (int index =0; index<path.size();index++)
//            printf("%d ",path[index]);
   }



}

//preprocess
std::tuple<bool,int,int> srcDst(char src[],char dst[])
{
    //check src and dst which is existed in fileName or not
    bool checkSrc=false,checkDst=false;
    int codeSrc,codeDst;
    for (auto building:codeBuilding)
    {
        if (src==building.first)
        {
            checkSrc=true;
            //printf("start location: %s\n",building.first.c_str());
            codeSrc=building.second;
        }
        if (dst==building.first)
        {
            checkDst=true;
            //printf("end location: %s\n",building.first.c_str());
            codeDst=building.second;
        }
    }
    if (checkSrc==false||checkDst==false)
        return std::make_tuple(0,0,0);



    return std::make_tuple(1,codeSrc,codeDst);

}

void checkCodeBuildings(char fileName[])
{
    //check code of buildings which is actually appeared in map or not
    FILE *fptr;
    std::vector <int> checkList;
    fptr=fopen(fileName,"r");
    if (fptr==NULL)
        printf("error file name");

    for (int i =0; i <rowNum;i++)
        for (int j=0; j<colNum;j++)
        {
            int value;
            fscanf(fptr,"%d",&value);
            if (value!=0&&value!=1)
                checkList.push_back(value);
        }
    sort(checkList.begin(),checkList.end());
    printf("Available codes: ");
    for(auto value :checkList)
        printf("%d ",value);
    printf("\n");
    fclose(fptr);


}

int findOrderNode(char fileName[],int code)
{
    //with code is given, finding row and column of building in map and return order of node
    FILE *fptr;
    fptr=fopen(fileName,"r");
    if (fptr==NULL)
        printf("error file name");
    for (int indexI=0; indexI<rowNum;indexI++)
        for (int indexJ=0; indexJ<colNum;indexJ++)
        {
            int value;
            fscanf(fptr,"%d",&value);
            if (value==code)
                return rowCol2Node(indexI,indexJ);
        }
    fclose(fptr);
    exit(0);

}

int rowCol2Node(int indexRow, int indexCol)
{
    //convert index of row and column in map to the order of node
    int orderNode= indexCol + colNum*indexRow;
    return orderNode;
}

std::pair<int,int> node2RowCol(int orderNode)
{
    int indexRow=orderNode/colNum;
    int indexCol=orderNode%colNum;
    return std::make_pair(indexRow,indexCol);
    //convert the order of node to index of row and column in map to
}

void listLocation(char fileName[])
{
    //save code of buildings in codeBuilding vector
    FILE *fptr;
    fptr = fopen(fileName,"r");
    int maxLines;
    fscanf(fptr,"%d",&maxLines);
    for (int index=0;index<maxLines;index++)
    {
        std::string building="";
        char ch;
        int value;

        fscanf(fptr,"%d",&value);
        //remove white space
        fscanf(fptr,"%c",&ch);
        while(true)
        {
            fscanf(fptr,"%c",&ch);
            if(ch=='\n')
                break;
            building+=ch;
        }
        codeBuilding.push_back(std::make_pair(building,value));
    }
    fclose(fptr);
}

void showPath(char mapFile[],char pathFile[])
{
    auto scaleSrc=1;
    auto startX=0,startY=210;
    cv::Mat srcImg = cv::imread(mapFile);
    cv::Mat mask = cv::imread(pathFile,cv::IMREAD_GRAYSCALE) < 200;

    resize(srcImg,srcImg,cv::Size(),scaleSrc,scaleSrc);
//    imwrite("VGU map 8k(resized).jpg",srcImg);
    resize(mask,mask,cv::Size(),7.02,7);

    cv::Mat blank(srcImg.size().height,srcImg.size().width,CV_8UC1,cv::Scalar(0));
    mask.copyTo(blank.rowRange(startY,startY+mask.size().height).colRange(startX,startX+mask.size().width));
    cv::Mat red;
    cv::cvtColor(blank, red, cv::COLOR_GRAY2BGR);
    red = (red - cv::Scalar(0, 0, 0)) ;
    cv::GaussianBlur(red,red,cv::Size(21,21),0,0);
    srcImg = srcImg - red;

//    imshow("imag1e",blank);
    cv::imshow("image", srcImg);
    cv::waitKey(0);
    cv::destroyAllWindows();

}













