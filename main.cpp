#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <dirent.h>
#include <termios.h>
#include <vector>
#include <cstring>
#include <stack>
#include <fcntl.h>
#include <iomanip>
#include <sys/wait.h>

using namespace std;
int isDirectory(const char* path);
int getcount();
void navigate();
void ncan();
void clearScreen();
void displaydir(string path);

stack<string> backw,forw; bool movef=false;
string forwpath,parentpath; bool back=false; 
long long total; bool present=false;
char parentArray[2000];
vector<string> store; int count=0; string path;
struct termios old_tio,new_tio;

int isDirectory(const char* path){                  //checking if the given path is directory or a file
    DIR* dir=opendir(path);
    if(dir!=NULL){
        closedir(dir);
        return 0;
    }
    return -1;
}

void display(string tempath){
    char *con_path=new char[tempath.length()+1];
    strcpy(con_path,tempath.c_str());
    DIR *dir;
    struct dirent *direntry=NULL;

    dir=opendir(con_path);
    if(dir==NULL){
        int pid=fork();
        if(pid==0){
            execl("/usr/bin/vi","vi",con_path,NULL);
        }
        else
            wait(NULL);
    }
    while((direntry=readdir(dir))!=NULL){
        struct stat stats;
        cout<<std::left<<std::setw(40)<<direntry->d_name<<"\t\t";

        string temp=tempath;
        temp=temp+"/"+string(direntry->d_name);

        char *newsrc=new char[temp.length()+1];
        strcpy(newsrc,temp.c_str());

        store.push_back(direntry->d_name);
        stat(newsrc,&stats);
        if((string(direntry->d_name)=="..") || (string(direntry->d_name)==".")){ cout<<endl; }
        else{
        cout<<stats.st_size<<"\t";

        switch(stats.st_mode & S_IFMT){
            case S_IFDIR : cout<<"d"; break;
            case S_IFREG : cout<<"-"; break;
            case S_IFBLK : cout<<"b"; break;
            case S_IFCHR : cout<<"c"; break;
        }

        if(stats.st_mode & S_IRUSR){
            cout<<"r";
        }
        else
            cout<<"-";
        if(stats.st_mode & S_IWUSR){
            cout<<"w";
        }
        else
            cout<<"-";
        if(stats.st_mode & S_IXUSR){
            cout<<"x";
        }
        else
            cout<<"-";
        if(stats.st_mode & S_IRGRP){
            cout<<"r";
        }
        else
            cout<<"-";
        if(stats.st_mode & S_IWGRP){
            cout<<"w";
        }
        else
            cout<<"-";
        if(stats.st_mode & S_IXGRP){
            cout<<"x";
        }
        else
            cout<<"-";
        if(stats.st_mode & S_IROTH){
            cout<<"r";
        }
        else
            cout<<"-";
        if(stats.st_mode & S_IWOTH){
            cout<<"w";
        }
        else
            cout<<"-";
        if(stats.st_mode & S_IXOTH){
            cout<<"x";
        }
        else
            cout<<"-";
        cout<<"\t";
        cout<<ctime(&stats.st_mtime);
        } 
    }
    closedir(dir);
    return;
}
void clearScreen(){                    //clearing the terminal screen
    cout<<"\033[2J\033[1;1H";
}
string getfilename(string path){
    int sz=path.size()-1; string res="";
    while(path[sz]!='/'){
        res=path[sz]+res;
        sz--;
    }
    return res;
}
void resncan(){
    tcsetattr(STDIN_FILENO,TCSANOW,&old_tio);
}
void delete_file(string path){
    char *src=new char[path.length()+1];
    strcpy(src,path.c_str());
    int sts=remove(src);
    if(sts!=0){
        cout<<"error occurred while deleting"<<endl;
    }
}
void delete_dir(string path){
    char *src=new char[path.length()+1];
    strcpy(src,path.c_str());
    DIR *d;
    struct dirent *dir;
    d=opendir(src);
    if(d){
        while((dir=readdir(d))!=NULL){
            if((string(dir->d_name)=="..") || (string(dir->d_name)==".")){ }
            else{
                string fpath=string(path)+"/"+string(dir->d_name);
                char *fsrc=new char[fpath.length()+1];
                strcpy(fsrc,fpath.c_str());
                struct stat sb;
                if(stat(fsrc,&sb)==-1){ }
                else{
                    if((S_ISDIR(sb.st_mode))){
                        delete_dir(fpath);
                    }
                    else{
                        delete_file(fpath);
                    }
                }
            }
        }
        closedir(d);
        int sts=rmdir(src);
    }
}
void create_file(string des){
    char *path=new char[des.length()+1];
    strcpy(path,des.c_str());
    int sts=open(path,O_RDONLY | O_CREAT,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(sts==-1){
        cout<<"error occurred while creating"<<endl;
    }
}
void create_dir(string des){
    char *src=new char[des.length()+1];
    strcpy(src,des.c_str());
    int sts=mkdir(src,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if(sts==-1){
        cout<<"error occurred while creating"<<endl;
    }    
}
void rename_file(string sname,string dname){
    /*sname=path+"/"+sname;
    cout<<sname<<endl;
    dname=path+"/"+dname;
    cout<<dname<<endl;*/
    char *src=new char[sname.length()+1];
    strcpy(src,sname.c_str());
    char *des=new char[dname.length()+1];
    strcpy(des,dname.c_str());
    int sts=rename(src,des);
    if(sts!=0){
        cout<<"error occurred while renaming"<<endl;
    }
}
bool searchfile(string tempath,string file){
    DIR *d;
    struct dirent *dir;
    char *src=new char[tempath.length()+1];
    strcpy(src,tempath.c_str());
    d=opendir(src);
    if(d){
        while((dir=readdir(d))!=NULL){
           if((string(dir->d_name)=="..") || (string(dir->d_name)==".")){ }
           else{
               string name=dir->d_name;
               //cout<<name<<endl;
               string fpath=string(src)+"/"+name;
               char *newp=new char[fpath.length()+1];
               strcpy(newp,fpath.c_str());

               struct stat sb;
				if (stat(newp, &sb) == -1)
				{
					perror("lstat");
				}
                else{
                    if((S_ISDIR(sb.st_mode))){
                        if(name==file){
                            //cout<<"True"<<endl; return;
                            present=true; break;
                        }
                        else{
                            present=searchfile(fpath,file);
                        }
                    }
                    else{
                        if(name==file){
                            //cout<<"True"<<endl; return;
                            present=true; break;
                        }
                    }
                }
           } 
        }
        //cout<<"False"<<endl;
        
    }
    return present;
}
void copyfile(string sor,string des){
    char temp[1024];
    int inp,outp,fread;
    
    char *src=new char[sor.length()+1];
    strcpy(src,sor.c_str());
    
    string file=getfilename(string(src));
    des=des+"/"+file;
    char *fin=new char[des.size()+1];
    
    strcpy(fin,des.c_str());
    inp=open(src,O_RDONLY);
    outp=open(fin,O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
    
    while((fread=read(inp,temp,sizeof(temp)))>0)
        write(outp,temp,fread);
}
void copydir(char *sor,char* des){
    string desf=string(des)+"/"+getfilename(string(sor));
    
    char *desti=new char[desf.length()+1];
    strcpy(desti,desf.c_str());
    
    int st=mkdir(desti,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if(st==-1)
        cout<<"error"<<endl;
    
    DIR *direc;
    struct dirent *dir;
    direc=opendir(sor);
    
    if(direc){
        while((dir=readdir(direc))!=NULL){
            if((string(dir->d_name)=="..")|| (string(dir->d_name)==".")){ }
            else{
                string fname=string(sor)+"/"+string(dir->d_name);
                char *newp=new char[fname.length()+1];
                strcpy(newp,fname.c_str());

                
                struct stat sb;
                if(stat(newp,&sb)==-1){}
                else{
                    if((S_ISDIR(sb.st_mode))){
                        copydir(newp,desti);
                    }
                    else{
                        copyfile(fname,desf);
                    }
                }
            }
        }
    }
}

void normalmode(){
    vector<string> cmd;
    char c; string s="";
    
    while(1){
    c=cin.get(); cout<<c; 
        if(c==127){
            cout<<"\b \b";
            continue;
        }
        else if(c==27){
            clearScreen();
            display(path);
            ncan();
        }
        else if(c==32){
            cmd.push_back(s);
            s="";
        }
        else if(c==10){
            cmd.push_back(s);
            s=""; 
            if(cmd[0]=="copy" || cmd[0]=="move"){
                if(cmd[0]=="move")
                    movef=true;
                else
                {
                    movef=false;
                }
                
                string des=cmd[cmd.size()-1];
                
                char *dest=new char[des.length()+1];
                strcpy(dest,des.c_str());                
                
                for(int i=1;i<cmd.size()-1;i++){
                    string file=cmd[i];
                    char *src=new char[file.length()+1];
                    strcpy(src,file.c_str());
                    if(isDirectory(src)==0){
                        copydir(src,dest);
                        if(movef)
                            delete_dir(file);
                    }
                    else{
                        copyfile(file,des);
                        if(movef)
                            delete_file(file);
                    }
                }
                cmd.clear();
                break;
            }
            else if(cmd[0]=="delete_file"){
                delete_file(cmd[1]);
                cmd.clear();
                break;
            }
            else if(cmd[0]=="delete_dir"){
                delete_dir(cmd[1]);
                cmd.clear();
                break;
            }
            else if(cmd[0]=="create_file"){
                string fname=cmd[1];
                string des=cmd[2]+"/"+fname;
                create_file(des);
                cmd.clear();
                break;
            }
            else if(cmd[0]=="create_dir"){
                string dirname=cmd[1];
                string des=cmd[2]+"/"+dirname;
                create_dir(des);
                cmd.clear();
                break;
            }
            else if(cmd[0]=="rename"){
                if(cmd.size()<3){
                    cout<<"not enough arguments"<<endl;
                    return;
                }
                rename_file(cmd[1],cmd[2]);
                cmd.clear();
                break;
            }
            else if(cmd[0]=="goto"){
                if(cmd.size()<2){
                    cout<<"not enough arguments"<<endl;                
                    return;
                }
                clearScreen();
                display(cmd[1]);
                backw.push(path);
                path=cmd[1];
                cmd.clear();
                ncan();
                break;
            }
            else if(cmd[0]=="search"){
                if(cmd.size()<2){
                    cout<<"not enough arguments"<<endl;                
                    return;
                }
                present=searchfile(path,cmd[1]);
                if(present){
                    cout<<"True"<<endl;
                    present=false;
                }
                else
                {
                    cout<<"False"<<endl;
                }
                
                cmd.clear();
                break;
                //bool pre=search(path,cmd[1]);
            }
        }
        else{
            s+=c;
        }

    }
            cout<<": ";
        normalmode();
}
void ncan(){ 
    tcgetattr(STDIN_FILENO,&old_tio);
    atexit(resncan);
    
    new_tio=old_tio;
    new_tio.c_lflag &= ~(ICANON | ECHO);
   
    tcsetattr(STDIN_FILENO,TCSANOW,&new_tio);
    char c; int pos; bool flag=true;
    while(1){
        char c;
        c=cin.get();
        long long index=store.size()-1;
        total=index+1;
       // backw.push(path);
        if(c=='A' && flag){
            count++;
            //cout<<count;
            cout<<"\033[1A";
        }
        else if(c=='B' && flag){
            count--;
            if(count<0)
                count=0;            
            cout<<"\033[1B";
        }
        else if(c==10 && flag){               //enter key press
            index-=(count-1);
            count=0;
            clearScreen();
            backw.push(path);
            path+="/"+store[index];
            display(path);
            ncan();
        }
        else if(c==68 && flag){                //left key press
        //    forwpath=path;
            if(!backw.empty()){
                forw.push(path);
                path=backw.top();
                backw.pop();
                int pos=path.size()-1;
                while(path[pos]!='/'){
                    pos--;
                }
                parentpath=path.substr(0,pos);
                //path=parentpath;
                clearScreen();
                display(path);
                ncan();
            }
        } 
        else if(c==127 && flag){                 //backspace press
        //    forwpath=path;
            forw.empty();
            backw.pop();
            int pos=path.size()-1;
            while(path[pos]!='/'){
                pos--;
            }
            parentpath=path.substr(0,pos);
            path=parentpath;
            clearScreen();
            display(path); 
            back=true;  
            ncan();         
        }
        else if(c==67 && flag){                     //right key press
            if(!back){
                if(!forw.empty()){
                    backw.push(path);
                    path=forw.top();
                    forw.pop();
                    int pos=path.size()-1;
                    while(path[pos]!='/'){
                        pos--;
                    }
                    parentpath=path.substr(0,pos);
                    clearScreen();
                    display(path);
                    ncan();
                }
            }
        }
        else if(c==104 || c==72 && flag){           //home key press
            backw.push(path);
            forw.empty();
            path="/home/parth";
            clearScreen();
            display(path);
            ncan();
        }
        else if(c==58){                           //enter command mode
            
            flag=false;
            clearScreen();
            display(path);
            cout<<"\n\n\n\n";
            cout<<": ";
            normalmode();
        }
    }
    return;
}
int main(){
    
    clearScreen();
    path=".";
    backw.push(path);
    forwpath=parentpath=path;
    display(".");
    ncan();
 
    return 0;
}