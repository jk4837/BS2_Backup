#include <stdio.h>
#include <stdlib.h>
#include <math.h>
int main()
{
    int i,size,pad;
    unsigned char c;
    unsigned char *si,*ei,*title;
    unsigned char ID[80],name[50],date[9],title0[73+3] = {0xa1,0xbb,' '};
    unsigned char file_path[78]={"A\\A1234567"};
    unsigned char *file = &(file_path[3]);
    FILE *fd_i,*fd_o;   // directory file
    FILE *fp_i,*fp_o;   // content file
    char dir[80]={""},full_path[80]={""},board[16]={""},opt[45]={""};

    do
    {
        printf("Please input .DIR file path: \n");
        scanf("%s",full_path);
        si = strstr(full_path,".DIR");
    }while(!si);
    if(full_path[0]=='\"')
        strncpy(dir,full_path+1,strlen(full_path)-2-4);
    else
        strncpy(dir,full_path,strlen(full_path)-4);
    printf("%s\n",dir);
    ei = strchr(full_path,'\\');
    while(strchr(ei+1,'\\'))
    {
        si = ei;
        ei = strchr(si+1,'\\');
    }
    strncpy(board,si+1,ei-si-1);
    printf("Board = %s\n",board);

    fd_i = fopen(full_path,"rb");
    if(!fd_i)
    {
           printf("Loading directory file Failed!\n");
           system("pause");
           return 0;
    }

    fseek(fd_i, 0, SEEK_END);
    size = ftell(fd_i);         // file size
    fseek(fd_i, 0, SEEK_SET);

    pad = log10(size/256)+1;

    strcpy(opt,"mkdir BBS_backup\\");
    strcat(strcat(opt,board),"\\content");
    system(opt);

    strcpy(opt,"BBS_backup\\");
    strcat(strcat(opt,board),"\\directory.txt");
    fd_o = fopen(opt,"w");

    if(!fd_o)
    {
        printf("Open writing file directory.txt Failed!\n");
        system("pause");
        return 0;
    }
    int TYPE = 0;
    do
    {
        printf("備份種類\n1.看板文章\n2.個人信件\n請選擇 : ");
        scanf("%d",&TYPE);
    }while(TYPE == 0);


    i=0;
    while(1)
    {
        fseek(fd_i, 12, SEEK_CUR);
        if(fread(file,sizeof(char),32,fd_i)<=0)
            break;
        fread(ID,sizeof(char),80,fd_i);
        si = strchr(ID,'.');    // jk4837@ptt.cc F
        if(si)
            *(si+1) = '\0';
        fread(name,sizeof(char),50,fd_i);
        fseek(fd_i, 3, SEEK_CUR);           // skip year
        fread(date,sizeof(char),6,fd_i);
        fread(title0+3,sizeof(char),73,fd_i);//++(square symbol) when ! Re

        if(!strncmp(title0+3,"Re: ",4))
        {
            // in order to edit "Re: " to "Re "
            title = title0+4;   // e: ...
            title[0] = 'R';
            title[1] = 'e';

        }
        else    // not Re: adding square symbol
        {
            title0[0] = 0xa1;
            title0[1] = 0xbb;
            title0[2] = ' ';
            title = title0;
        }

        if(TYPE==1)
            file_path[0] = file[7];
        else
            file_path[0] = '@';
        file_path[1] = '\\';
        file_path[2] = '\\';    // redundant
        sprintf(full_path,"%s%s",dir,file_path);
        fp_i = fopen(full_path,"r");
        if(!fp_i)
        {
            printf("Can't find %s content!\n",full_path);
            system("pause");
        }
        else
        {
            fprintf(fd_o,"%6d %s %-13s %-46s\n",++i,date,ID,title);
            // \/:*?"<>| can't be file name
            date[2] = '_';
            for(si=title+2;*si!='\0';si++)
            {
                if((unsigned char)(*si)>0x7f)     // Chinese
                {
                    si++;
                    if((*(si)>=0x40 && *(si)<=0x7E)||(*(si)>=0xA1 && *(si)<=0xFE))
                        ;
                    else
                    {
                        *(si-1) = ' ';
                        *(si) = ' ';
                    }
                }
                else if(*si=='\\' || *si=='/' || *si==':' || *si=='*' || *si=='?' || *si=='\"' || *si=='<' || *si=='>' || *si=='|')
                    *si = ' ';
            }

            memset(file_path,'\0',sizeof(file_path));
            // Here you can change the filename's formate.
            sprintf(file_path,"BBS_backup\\%s\\content\\%0*d  .  %s  .  %s.txt",board,pad,i,date,title);
            fp_o = fopen(file_path,"wb");
            if(!fp_o)
            {
                printf("Writing to file \"%s\" Failed!\n",file_path);
                system("pause");
            }
            c = fgetc(fp_i);
            while(c!=(unsigned char)EOF)
            {
                if(c==0x1b)                 // *
                {
                    c = fgetc(fp_i);
                    if(c==0x5b)             // [
                    {
                        while(c!=0x6d)      // m
                            c = fgetc(fp_i);
                    }
                    if(c==0x2a)             // push content control char
                        c = fgetc(fp_i);
                }
                else if(c==0x0a)
                {
                    fputc(0x0d,fp_o);
                    fputc(0x0a,fp_o);
                }
                else
                {
                    fputc(c,fp_o);
                    if((unsigned char)c>0x7f)
                    {
                        c = fgetc(fp_i);
                        fputc(c,fp_o);
                    }
                }
                c = fgetc(fp_i);
            }
            fclose(fp_o);
            fclose(fp_i);
        }
    }
    fclose(fd_o);
    fclose(fd_i);
    printf("Finish!\n");
    system("pause");
    return 0;
}
