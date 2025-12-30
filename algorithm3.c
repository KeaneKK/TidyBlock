#include <stdio.h>
#include <stdlib.h>

// 定义点结构
typedef struct {
    int x;
    int y;
} Point;

// 定义矩形区域结构
typedef struct {
    int xl; // x-low
    int yl; // y-low
    int xh; // x-high
    int yh; // y-high
    int xcenter;
    int ycenter;
    int maxcover;
    int maxcoverOfp;
} Rec;

// qsort的比较函数
int compare_int(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

// 去重并排序一个整数数组
int unique_sort(int *arr, int n) {
    if (n == 0) return 0;
    qsort(arr, n, sizeof(int), compare_int);
    int j = 0;
    for (int i = 1; i < n; i++) {
        if (arr[i] != arr[j]) {
            j++;
            arr[j] = arr[i];
        }
    }
    return j + 1;
}

int iscover(Rec r,Point p,int halflength,int halfwidth)
{
    if(p.x>=(r.xcenter-halflength)&&p.x<=(r.xcenter+halflength)&&p.y>=(r.ycenter-halfwidth)&&p.y<=(r.ycenter+halfwidth)) return 1;
    return 0;
}
int iscoverOfp(int px,int py,Point p,int halflength,int halfwidth)
{
    if(p.x>=(px-halflength)&&p.x<=(px+halflength)&&p.y>=(py-halfwidth)&&p.y<=(py+halfwidth)) return 1;
    return 0;
}

int main() {
    // 输入案例
    Point points[] = {{2, 2},{2,4},{6,4},{6,6},{4, 6}};
    int num_points = sizeof(points) / sizeof(points[0]);
    int length = 2;
    int width = 2;

    printf("输入点集:\n");
    for (int i = 0; i < num_points; i++) {
        printf("(%d, %d) ", points[i].x, points[i].y);
    }
    printf("\n");
    printf("矩形A的尺寸: length = %d, width = %d\n\n", length, width);

    // 1. 收集所有可能的矩形A中心坐标范围
    // 对于每个点，矩形A要覆盖它，中心点必须在特定范围内
    int *candidate_x = malloc(2 * num_points * sizeof(int));
    int *candidate_y = malloc(2 * num_points * sizeof(int));
    if (candidate_x == NULL || candidate_y == NULL) {
        printf("内存分配失败\n");
        return 1;
    }

    int half_length = length / 2;
    int half_width = width / 2;

    for(int i=0;i< num_points;i++)
    {
        candidate_x[2*i]=points[i].x-half_length;
        candidate_x[2*i+1]=points[i].x+half_length;
        candidate_y[2*i]=points[i].y-half_width;
        candidate_y[2*i+1]=points[i].y+half_width;
    }

    int num_unique_x = unique_sort(candidate_x, 2 * num_points);
    int num_unique_y = unique_sort(candidate_y, 2 * num_points);
   // Point pOfrec[num_unique_x-1][num_unique_y-1];//只考虑每个矩形的左下角点
    //test
    for(int i=0;i<num_unique_x;i++)
    {
        printf("x=%d ",candidate_x[i]);
    }
    printf("\n");
    for(int i=0;i<num_unique_y;i++)
    {
        printf("y=%d ",candidate_y[i]);
    }
    printf("\n");



    Rec rect[num_unique_y-1][num_unique_x-1];//从左往右，从下到上。
    int maxcover=0;
    int maxiOfrec=-1;
    int maxjOfrec=-1;
    for(int j=0;j<num_unique_y-1;j++)
    {
        for(int i=0;i<num_unique_x-1;i++)
         {
            rect[j][i].xl=candidate_x[i];
            rect[j][i].xh=candidate_x[i+1];
            rect[j][i].yl=candidate_y[j];
            rect[j][i].yh=candidate_y[j+1];
            rect[j][i].maxcover=0;
            rect[j][i].maxcoverOfp=0;
            rect[j][i].xcenter=(rect[j][i].xl+rect[j][i].xh)/2;
            rect[j][i].ycenter=(rect[j][i].yl+rect[j][i].yh)/2;
            for(int k=0;k<num_points;k++)
            {
                if(iscover(rect[j][i],points[k],half_length,half_width)==1) 
                {
                    rect[j][i].maxcover++;
                }
                if(iscoverOfp(rect[j][i].xl,rect[j][i].yl,points[k],half_length,half_width)==1)
                {
                    rect[j][i].maxcoverOfp++;
                }
            }
            //test
            printf("j=%d i=%d xcenter=%d ycenter=%d cover=%d\n",j,i,rect[j][i].xcenter,rect[j][i].ycenter,rect[j][i].maxcover);

            if(rect[j][i].maxcover>maxcover)
            {
                maxiOfrec=i;
                maxjOfrec=j;
                maxcover=rect[j][i].maxcover;
            }
            if(rect[j][i].maxcoverOfp>maxcover)
            {
                maxiOfrec=i;
                maxjOfrec=j;
                maxcover=rect[j][i].maxcoverOfp;
            }


            

         }
    }

     //test
            printf("maxcover=%d j=%d i=%d \n",maxcover,maxjOfrec,maxiOfrec);

 
    
   


    return 0;
}