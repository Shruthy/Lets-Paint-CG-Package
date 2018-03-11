#include<stdio.h>
#include<conio.h>
#include<graphics.h>
#include<dos.h>
#include<stdlib.h>
#include<math.h>
#include<process.h>

#define MIN_X 100      //basically working area
#define MAX_X 627
#define MIN_Y 40
#define MAX_Y 370
#define BUTTON_SIZE 20 // max. number of buttons
#define PI 3.14

int xr1,yr1,xr2,yr2;    //after drawin rect its coordinates are in this
int xl1,yl1,xl2,yl2;    //after drawin line its coordinate are in this
int current_x,current_y;
int Current_Color=BLACK; // to  Store the current color
int Current_Button=-1;//to store current button pressed
int Prev_Button=-1;
int mousex,mousey,prevx,prevy;
int LeftPressed=0,RightPressed=0;//left and right buttons of mouse initilalized
union REGS regs;
struct
{
	int minx,miny; //boundary information
	int maxx,maxy;
	int butt_no; //button number
	char* desc;
}button[BUTTON_SIZE];

struct
{
	int xmin,ymin;
	int xmax,ymax;
	int color;
}colorbutton[16];
typedef struct coordinate   //for line clipping
{
    int x,y;
    char code[4];
}PT;

int initmouse()
{
  regs.x.ax=0;
  int86(0x33,&regs,&regs);
  return(regs.x.bx);
}
void startmouse(int x,int y)  //sets the cursor position to the x & y co-ordn
{
  regs.x.ax=4;
  regs.x.cx=x;
  regs.x.dx=y;
  int86(0x33,&regs,&regs);
}
void showmouse()
{
  regs.x.ax=1;
  int86(0x33,&regs,&regs);
}
void hidemouse()
{
  regs.x.ax=2;
  int86(0x33,&regs,&regs);
}
void getxy()
{
  regs.x.ax=3;              //get mouse status using 03h of int 33h
  int86(0x33,&regs,&regs);
  prevx=mousex;
  prevy=mousey;
  if(regs.x.bx&1)           //LSB of reg BX
	 LeftPressed=1;
  else
	 LeftPressed=0;
  mousex=regs.x.cx;         //(cx,dx)=(x,y)
  mousey=regs.x.dx;
  if(regs.x.bx&2)
	 RightPressed=1;
  else
	 RightPressed=0;
}

void clear_status()
{
	 setfillstyle(1,7);
	 bar(11,453,499,471);
}
void show_status(char* str)
{
	 int color=getcolor();
	 int i;
	 clear_status();
	 setcolor(0);
	 outtextxy(15,460,str);
	 setcolor(color);
}
void disp_cordn()   //to display the coordinates
{
	char x[5],y[5];
	int color;
	if(prevx!=mousex||prevy!=mousey)
	{
	if((mousex>MIN_X+1)&&(mousex<MAX_X-1)&&(mousey>MIN_Y+1)&&(mousey<MAX_Y-1))
	{
		 settextstyle(DEFAULT_FONT,0,0);
		 sprintf(x,"x=%d",mousex-MIN_X-2);
		 sprintf(y,"y=%d",mousey-MIN_Y-2);
		 color=getcolor();
		 setfillstyle(1,LIGHTGRAY);
		 bar(531,441,MAX_X-31,471);  //refresh screen to display time
		 setcolor(BLACK);
		 outtextxy(540,442,x);
		 outtextxy(540,460,y);
		 setcolor(color);
	}
	else
	{   //if outside the working window then simply clear the screen
	 setfillstyle(1,LIGHTGRAY);
	 bar(531,441,MAX_X-31,471);
	}
	}
}
void restrictmouse(int minx,int miny,int maxx,int maxy)
{
	 regs.x.cx=minx;
	 regs.x.dx=maxx;
	 regs.x.ax=0x7;
	 int86(0x33,&regs,&regs);
	 regs.x.cx=miny;
	 regs.x.dx=maxy;
	 regs.x.ax=0x8;
	 int86(0x33,&regs,&regs);
}
void icon(int xmin,int ymin,int xmax,int ymax,int status)
{
	int UP=WHITE,DOWN=DARKGRAY;
	if(status == 0)
	{
		UP=DARKGRAY; DOWN=WHITE;
	}
	setcolor(UP);
	line(xmax,ymin,xmax,ymax);
	line(xmax,ymax,xmin,ymax);
	setcolor(DOWN);
	line(xmin,ymin,xmin,ymax);
	line(xmin,ymin,xmax,ymin);
}
void draw_button(int num)
{
	hidemouse();
	icon(button[num].minx,button[num].miny,button[num].maxx,button[num].maxy,0);
	showmouse();
}
void undraw_button(int num)
{
	hidemouse();
	icon(button[num].minx,button[num].miny,button[num].maxx,button[num].maxy,1);
	showmouse();
}
void init_button(int num,int x_min,int y_min,int x_max,int y_max,char* desc)
{
	button[num].minx=x_min;
	button[num].miny=y_min;
	button[num].maxx=x_max;
	button[num].maxy=y_max;
	button[num].desc=(char*)malloc(strlen(desc)+1);//string for ShowStatus()
	if(button[num].desc==NULL)
	{
		cleardevice();
		printf("\n No MEMORY ");
	}
	strcpy(button[num].desc,desc);
}
int check_mouse_pos(int minx,int miny,int maxx,int maxy)
{
	if(mousex>minx && mousex<maxx && mousey>miny && mousey<maxy)
		return 1;
	return 0;
}
int check_if_button_pressed()
{
	int i;
	for(i=0;i<BUTTON_SIZE;i++)
	{
		if(check_mouse_pos(button[i].minx,button[i].miny,button[i].maxx,button[i].maxy))
		{
			if(LeftPressed && i!=Current_Button) //check if button pressed
			{
				if(Current_Button != -1)
					draw_button(Current_Button);
				Prev_Button=Current_Button;
				Current_Button=i;
				undraw_button(Current_Button);
			}
			if(prevx!=mousex||prevy!=mousey)//if button selected is not change, no update
				show_status(button[i].desc);
			return Current_Button;
		}
	}
	clear_status();
	return -1;//no button pressed
}
int check_if_menu()
{
	getxy();
	if(check_mouse_pos(10,13,30,23))//check if mouse position on "clear" icon
	{
		getxy();
		if(LeftPressed)
		{
			draw_button(Current_Button);
			setfillstyle(SOLID_FILL,WHITE);
			bar(MIN_X+2,MIN_Y+2,MAX_X-2,MAX_Y-2);
			return 1;
		}
	}
	clear_status();
	return 0;
}
void clear_window()  //sets the working window to WHITE color
{
	setfillstyle(SOLID_FILL,WHITE);
	bar(MIN_X+2,MIN_Y+2,MAX_X-2,MAX_Y-2);
}

void beep()
{
	 sound(1000);
	 delay(10);
	 nosound();
}
char* readline(char* msg)
{
  char* Line;
  char  temp[40],Disp_Line[60],ch;
  int i=0,length=0;
  int max=((475-12)-strlen(msg)*8)/8;	// find maximum number of characters
  clear_status();
  setcolor(0);
  outtextxy(15,460,msg);     //prints the msg in status box
  strcpy(Disp_Line,msg);
  temp[0] = 0;
  ch=getch();
  while((ch!=27)&& (ch!=13))
  {
	switch (ch)
	{
		case '\b':if(i==0)
						beep();
					 else
					 {
						i--;
						clear_status();
						length=strlen(Disp_Line);
						Disp_Line[length-1]='\0';
						setcolor(0);
						outtextxy(15,460,Disp_Line);
					 }
					 break;
		default:if(i > max)
					 beep();
				  else
				  {
					 length=strlen(Disp_Line);
					 Disp_Line[length]=ch;        //copy next char into Disp_Line
					 Disp_Line[length+1]='\0';
					 outtextxy(15,460,Disp_Line);
					 temp[i++]=ch;
					}
					break;
	}
	ch=getch();
  }
  temp[i]='\0';
  clear_status();
  if(ch==27)
	return NULL;
  else
  {
	 Line=(char*)malloc(strlen(temp)+1);
	 strcpy(Line,temp);
  }
  return Line;
}
void draw(int x,int y)
{
	if(x>MIN_X+2 && x<MAX_X-2 && y>MIN_Y+2 && y<MAX_Y-2)
		putpixel(x,y,Current_Color);
}
void putcircle(int x,int y,int xc,int yc)
{
 draw(xc+x,yc+y);
 draw(xc-x,yc+y);
 draw(xc+x,yc-y);
 draw(xc-x,yc-y);
 draw(xc+y,yc+x);
 draw(xc-y,yc+x);
 draw(xc+y,yc-x);
 draw(xc-y,yc-x);
}
void b_algo(int xc,int yc,int rad)
{
  int x=0,y=rad,p=3-2*rad;
  if(rad<3)
	 return;
  hidemouse();
  putcircle(x,y,xc,yc);
  while(x<=y)
  {
	 putcircle(x,y,xc,yc);
	 if(p<0)
		p+=4*x+6;
	 else
	 {
		p+=4*(x-y)+10;
		y--;
	 }
	 x++;
	 if(x==y)
			putcircle(x,y,xc,yc);
  }
  showmouse();
}
void bcircle()
{
	int x,y;
	getxy();
	x=mousex;y=mousey;
	if(LeftPressed)
	{
		while(LeftPressed)
		{
		  getxy();
		  disp_cordn();
		  if(prevx==mousex&&prevy==mousey)
			continue;
		  hidemouse();
		  showmouse();
		  getxy();
		  hidemouse();
		  setfillstyle(SOLID_FILL,WHITE);
		  bar(MIN_X+2,MIN_Y+2,MAX_X-2,MAX_Y-2); //new sheet
		  b_algo(x,y,abs(x-mousex)<abs(y-mousey)?abs(y-mousey):abs(x-mousex));
		  showmouse();
		  restrictmouse(3,3,637,477);
		 }
	}showmouse();
}
void drawline(int x1,int y1,int x2,int y2)
{
	int x,y,xend,yend,d;
	int dx=abs(x1-x2),dy=abs(y1-y2);
	hidemouse();
	if(dx>dy)
	{
		d=2*dy-dx;
		if(x1<x2)
		{
			x=x1;	y=y1;
			xend=x2;	yend=y2;
		}
		else
		{
			x=x2;	y=y2;
			xend=x1;	yend=y1;
		}
		while(x<=xend)
		{
			if(x>MIN_X+2 && x<MAX_X-2 && y>MIN_Y+2 && y<MAX_Y-2)
				putpixel(x,y,Current_Color);
			if(d<0)	d+=2*dy;
			else
			{
				d+=2*(dy-dx);
				if(y<yend)	y++;
				else	y--;
			}
			x++;
		}
	}
	else
	{
		d=2*dx-dy;
		if(y1<y2)
		{
			y=y1;	x=x1;
			yend = y2; xend=x2;
		}
		else
		{
			x=x2;	y=y2;
			xend=x1;	yend=y1;
		}
		while(y<=yend)
		{
			if(x>MIN_X+2 && x<MAX_X-2 && y>MIN_Y+2 && y<MAX_Y-2)
				putpixel(x,y,Current_Color);
			if(d<0)	d+=2*dx;
			else
			{
				d+=2*(dx-dy);
				if(x<xend)	x++;
				else	x--;
			}
			y++;
		}
	}
	showmouse();
}
void draw_line()
{
	int x,y;
	getxy();
	x=mousex;y=mousey;
	if(LeftPressed)
	{
		while(LeftPressed)
		{
		  getxy();
		  disp_cordn();
		  if(prevx==mousex&&prevy==mousey)
			continue;
		  hidemouse();
		 //putback();
		  showmouse();
		  getxy();
		  hidemouse();
		  setfillstyle(SOLID_FILL,WHITE);
		  bar(MIN_X+2,MIN_Y+2,MAX_X-2,MAX_Y-2); //new sheet
		  drawline(x,y,mousex,mousey);
		  showmouse();
		  restrictmouse(3,3,637,477);
		 }
		 xl1=x; yl1=y;
		 xl2=mousex; yl2=mousey;
	}
}
void line_arnd_circle(int xc,int yc,int r)
{
	int x,y,l=r+20;
	float tet=0;
	hidemouse();
	do
	{
		clear_window();
		setcolor(Current_Color);
		b_algo(xc,yc,r);
		x=xc+r*cos(tet);
		y=yc+r*sin(tet);
		drawline(x-l*cos(tet+(M_PI/2)),y-l*sin(tet+(M_PI/2)),x+l*cos(tet+(M_PI/2)),y+l*sin(tet+(M_PI/2)));
		tet+=0.1;
		delay(20);
		getxy();
	} while(RightPressed == 0);
	showmouse();
}
void rotate_line()
{
	int x,y;
	getxy();
	x=mousex;y=mousey;
	if(LeftPressed)
	{
		while(LeftPressed)
		{
			disp_cordn();
			restrictmouse(MIN_X+2,MIN_Y+2,MAX_X-2,MAX_Y-2);
			getxy();
		}
		restrictmouse(3,3,637,477);
		hidemouse();
		line_arnd_circle(x,y,abs(x-mousex)<abs(y-mousey)?abs(y-mousey):abs(x-mousex));
		showmouse();
	}
}
void algospiral(int xc,int yc,int max)
{
	 int x1,y1,x2,y2;
	 float theta,dtheta,radius;
	 theta=0.0;
	 radius=0.0;
	 dtheta=5.0*5-PI/max;
	 x1=xc;
	 y1=yc;
	 while(radius<max)
	 {
		x2=xc+radius*cos(theta);
		y2=yc+radius*sin(theta);
		if(x1>MIN_X && x2>MIN_X && y1>MIN_Y && y2>MIN_Y)
		{
			if(x1<MAX_X && x2<MAX_X && y1<MAX_Y && y2<MAX_Y)
			{
				line(x1,y1,x2,y2);
			}
		}
		x1=x2;
		y1=y2;
		radius+=0.2;
		theta+=dtheta;
	 }
}
void drawspiral()
{
	 int x1,x2,y1,y2,i;
	 int rad1,rad2;
	 int color=15-Current_Color;
	 getxy();
	 x1=mousex;
	 y1=mousey;
	 setcolor(color);
	 setwritemode(XOR_PUT);
	 while(LeftPressed)
	 {
		disp_cordn();
		restrictmouse(MIN_X+2,MIN_Y+2,MAX_X-2,MAX_Y-2);
		x2=mousex;
		y2=mousey;
		while(x2==mousex && y2==mousey)
		{
		 getxy();
		 mousex=mousex-(MIN_X+2);
		 mousey=mousey-(MIN_Y+2);
		}
		rad1=abs(x2-x1);
		if(rad1<1)
			rad1=1;
		rad2=abs(mousex-x1);
		if(rad2<1)
			rad2=1;
		hidemouse();
		algospiral(x1,y1,rad1);
		algospiral(x1,y1,rad2);
		showmouse();
	 }
	 setcolor(Current_Color);
	 setwritemode(COPY_PUT);
	 algospiral(x1,y1,rad2);
	restrictmouse(3,3,637,477);
}
void hermite(int x1,int y1)
{
	int x4,y4;
	double xt,yt,t,r1,r2;
	clear_window();
	x4=mousex; y4=mousey;
	r1=abs(x4-x1);
	r2=abs(y4-y1);
	for(t=0;t<1;t+=0.001)
	{
	xt=(2*t*t*t-3*t*t+1)*x1+(-2*t*t*t+3*t*t)*x4+(t*t*t-2*t*t+t)*r1+(t*t*t-t*t)*r2;
	yt=(2*t*t*t-3*t*t+1)*y1+(-2*t*t*t+3*t*t)*y4+(t*t*t-2*t*t+1)*r1+(t*t*t-t*t)*r2;
	if((xt>(MIN_X + 2) &&xt< (MAX_X - 2))&&(yt>(MIN_Y + 2)&&yt<(MAX_Y - 2)))
		putpixel(xt,yt,Current_Color);
	}
}
void hermite_curve()
{
	int x,y;
	showmouse();
	getxy();
	x=mousex;y=mousey;
	disp_cordn();
	if(LeftPressed)
	{
	while(LeftPressed)
	{
	  getxy();
	  disp_cordn();
	  if(prevx==mousex&&prevy==mousey)
		continue;
	  hidemouse();
		 //putback();
	  showmouse();
	  getxy();
	  hidemouse();
	  setfillstyle(SOLID_FILL,WHITE);
	  bar(MIN_X+2,MIN_Y+2,MAX_X-2,MAX_Y-2); //new sheet
	  hermite(x,y);
	  showmouse();
	  delay(10);
	  restrictmouse(3,3,637,477);
	}
	 }
}
void drawrectangle()
{
  int color=15-Current_Color;
  int current_x,current_y;
  int x,y;
  setcolor(color);
  setwritemode(XOR_PUT);
  x=mousex;
  y=mousey;
  while(LeftPressed)
  {
	disp_cordn();
	current_x=mousex;
	current_y=mousey;
	while(current_x==mousex && current_y==mousey)
		 getxy();
	hidemouse();
	rectangle(x,y,current_x,current_y); //while dragging the mouse the rect is erased until left button is released
	rectangle(x,y,mousex,mousey);    //draws the rectangle
	showmouse();
  }
  setwritemode(COPY_PUT);
  setcolor(Current_Color);
  xr1=x;
  yr1=y;
  xr2=mousex;
  yr2=mousey;
  //Sets the writing mode for line drawing in graphics mode
  //when XOR_PUT is set it replaces whatever is on screen
  //i.e if line is drawn and we again draw a line at same place then line
  //is deleted not just overwriten again.
}

PT setcode(PT p,int xmin,int ymin,int xmax,int ymax)
{
	 if(p.y<ymin)
	p.code[0]='1';
	 else
	p.code[0]='0';
	 if(p.y>ymax)
	p.code[1]='1';
	 else
	p.code[1]='0';
	 if(p.x>xmax)
	p.code[2]='1';
	 else
	p.code[2]='0';
	 if(p.x<xmin)
	p.code[3]='1';
	 else
	p.code[3]='0';
	 return p;
}

int visibilty(PT p1,PT p2)
{
	 int i,flag=0;
	 for(i=0;i<4;i++)
	if(p1.code[i]!='0'||p2.code[i]!='0')
		 flag=1;
	 if(flag==0)
	return 0; //Completly Visible..
	 for(i=0;i<4;i++)
	if((p1.code[i]==p2.code[i])&&(p1.code[i]=='1'))
		 flag=2;
	 if(flag==2)
	return 1;    //Completly Invisible
	 return 2;        //Partially Visible
}

PT resetendpt(PT p1,PT p2,int xmin,int ymin,int xmax,int ymax)
{
	 int x,y;
	 float m,k;
	 if(p1.x<xmin)
	x=xmin;
	 if(p1.x>xmax)
	x=xmax;
	 if(p1.x<xmin||p1.x>xmax)
	 {
	m=(float)(p2.y-p1.y)/(p2.x-p1.x);
	k=m*(x-p1.x)+p1.y;
	p1.x=x;
	p1.y=k;
	if(p1.y>=ymin||p1.y<=ymax)
		 return p1;
	 }
	 if(p1.y<ymin)
	y=ymin;
	 if(p1.y>ymax)
	y=ymax;
	 if(p1.y<ymin||p1.y>ymax)
	 {
	m=(float)(p2.y-p1.y)/(p2.x-p1.x);
	k=((y-p1.y)/m)+p1.x;
	p1.x=k;
	p1.y=y;
	return p1;
	 }
	 else
	return p1;
}

void drawlineclip(PT p1,PT p2)
{
    line(p1.x,p1.y,p2.x,p2.y);
}

void cohen(int x0,int y0,int x1,int y1,int xmin,int ymin,int xmax,int ymax)
{
    PT p1,p2;
    int v;
    p1.x=x0;
    p1.y=y0;
    p2.x=x1;
    p2.y=y1;
    p1=setcode(p1,xmin,ymin,xmax,ymax);
    p2=setcode(p2,xmin,ymin,xmax,ymax);
    v=visibilty(p1,p2);
    setfillstyle(1,WHITE);
	 //bar(101,31,634,399);
	 bar(MIN_X+2,MIN_Y+2,MAX_X-2,MAX_Y-2);
	 setcolor(Current_Color);
    rectangle(xmin,ymin,xmax,ymax);
    switch(v)
    {
	case 0:	drawlineclip(p1,p2); //Completly Visible
		break;
	case 1: break; //Completly Invisible..
	case 2: p1=resetendpt(p1,p2,xmin,ymin,xmax,ymax);
		p2=resetendpt(p2,p1,xmin,ymin,xmax,ymax);
		drawlineclip(p1,p2); //Partially Visible..
    }
}

void exchange(int *v1,int *v2)
{	 // to swap variables
	int temp;
	temp=*v1;
	*v1=*v2;
	*v2=temp;
}

void clip()
{
	 int x,y,x0,y0,x1,y1;  //for line
    int x3,y3;
	 clear_status();
	 showmouse();
	 show_status("Draw the Line to be clipped");
    getxy();
    x=mousex;
    y=mousey;
    if(x<MIN_X+2||x>MAX_X-2||y>MAX_Y-2||y<MIN_Y+2)
		return;
	 while(!LeftPressed)
	 {
		getxy();
		disp_cordn();
	 }
	 hidemouse();
	 draw_line();
	 x0=xl1;
	 y0=yl1;
	 x1=xl2;
	 y1=yl2;
	 clear_status();
	 show_status("Draw the Clipping Window");
	 showmouse();
	 getxy();
	 while(!LeftPressed)
	 {
		getxy();
		disp_cordn();
	 }
	 hidemouse();
	 drawrectangle();
	 x=xr1;
	 y=yr1;
	 x3=xr2;
	 y3=yr2;
	 if(x>x3)
		exchange(&x,&x3);
	 if(y>y3)
		exchange(&y,&y3);
	 cohen(x0,y0,x1,y1,x,y,x3,y3);
	 showmouse();
}

void wheel(int xc,int yc,int r)
{
	float tet=0;
	clear_status();
	show_status("Right click mouse to terminate");
	hidemouse();
	do
	{
		clear_window();
		setcolor(Current_Color);
		b_algo(xc,yc,r);
		if(xc > MAX_X-2)  //To rotate the wheel n move it horizontally
			xc=MIN_X+2;
		xc++;
		drawline(r*cos(tet)+xc,r*sin(tet)+yc,-r*cos(tet)+xc,-r*sin(tet)+yc);
		drawline(r*cos(tet+M_PI/2)+xc,r*sin(tet+M_PI/2)+yc,-r*cos(tet+M_PI/2)+xc,-r*sin(tet+M_PI/2)+yc);
		delay(10);
		tet+=0.1;
		getxy();
	}while(RightPressed == 0);
	showmouse();
}
void drawwheel()
{
	int x,y;
	restrictmouse(MIN_X+2,MIN_Y+2,MAX_X-2,MAX_Y-2);
	getxy();
	x=mousex;y=mousey;
	if(LeftPressed)
	{
		disp_cordn();
		restrictmouse(3,3,637,477);
		hidemouse();
		wheel(x,y,50);
		showmouse();
	 }
}
void plot(int x,int y,int xc,int yc,float f1,float f2)
{
	draw(xc+x*f1,yc+y*f2);
	draw(xc+x*f1,yc-y*f2);
	draw(xc-x*f1,yc+y*f2);
	draw(xc-x*f1,yc-y*f2);
	draw(xc+y*f1,yc+x*f2);
	draw(xc+y*f1,yc-x*f2);
	draw(xc-y*f1,yc+x*f2);
	draw(xc-y*f1,yc-x*f2);
}
void bellipse(int xc,int yc,int r1,int r2)
{
	int x,y,p;
	float f1=1,f2=1;
	if(r1<r2)
		f1=(float)r1/r2;
	else
		f2=(float)r2/r1;
	x=0;
	y = (r1>r2)?r1:r2;
	p = 3-2*y;
	while(x<y)
	{
		plot(x,y,xc,yc,f1,f2);
		if(p<0)	p+=4*x+6;
		else
		{
			p+=4*(x-y)+10;
			y--;
		}
		x++;
	}
}
void drawellipse(int xc,int yc)
{
	int r1,r2;
	hidemouse();
	r1=abs(xc-mousex),r2=abs(yc-mousey);
	if(r1!=0&&r2!=0)  //else floating point error
		bellipse(xc,yc,abs(xc-mousex),abs(yc-mousey));
	showmouse();
	delay(10);
}
void draw_ellipse()
{
	int x,y;
	getxy();  disp_cordn();
	x=mousex;y=mousey;
	if(LeftPressed)
	{
		while(LeftPressed)
		{
			disp_cordn();
			restrictmouse(MIN_X+2,MIN_Y+2,MAX_X-2,MAX_Y-2);
			getxy();
		}
		restrictmouse(3,3,637,477);
		hidemouse();
		drawellipse(x,y);
		showmouse();
	}
}
void segmentation()
{
	LeftPressed =0; RightPressed =0;
	do
	{
		getxy();
	}while(!LeftPressed);
	if(check_mouse_pos(MIN_X+2,MIN_Y+2,MAX_X-2,MAX_Y-2) && LeftPressed == 1)
	{
		hidemouse();
		setfillstyle(SOLID_FILL,BLACK);
		bar(MIN_X+2,MIN_Y+2,MAX_X-2,MAX_Y-2);
		setcolor(WHITE);
		line(275,325,425,325);//bottom
		delay(50);
		line(275,325,225,275);//left slant
		delay(50);
		line(425,325,475,275);//right slant
		delay(50);
		line(225,275,475,275);//top
		delay(100);
		line(245,295,455,295);//inner top
		delay(50);
		line(275,275,300,250);//small slant left
		delay(50);
		line(300,250,400,250);//small top
		delay(100);
		line(400,250,425,275);//small slant
		delay(50);
		rectangle(330,150,340,250);
		setfillstyle(SOLID_FILL,RED);
		setcolor(RED);
		outtextxy(350,300,"BLACK PEARL");
		delay(100);
		bar(331,155,339,157);
		bar(331,162,339,164);
		delay(100);
		setfillstyle(SOLID_FILL,12);
		bar(341,150,372,155);
		setfillstyle(SOLID_FILL,WHITE);
		bar(341,155,372,160);
		setfillstyle(SOLID_FILL,GREEN);
		bar(341,160,372,165);
		setcolor(BLUE);
		circle(355,157.5,2);
	}
	showmouse();
}
void mul(float a[10][10],float d[10][10])
{
	 char m1[4];
	 int i,j,k,xc,yc,zc;
	 float c[10][10];
	 for(i=0;i<1;i++)
	for(j=0;j<4;j++)
	{
		 c[i][j]=0;
		 for(k=0;k<4;k++)
		c[i][j]+=a[i][k]*d[k][j];
	}
	xc=c[0][0],yc=c[0][1],zc=c[0][2];
	outtextxy(570,100,"x=");
	outtextxy(570,120,"y=");
	outtextxy(570,140,"z=");
	sprintf(m1,"%d",(xc));
	outtextxy(600,100,m1);
	sprintf(m1,"%d",(yc));
	outtextxy(600,120,m1);
	sprintf(m1,"%d",(zc));
	outtextxy(600,140,m1);
	if(xc+MIN_X>MIN_X&&yc+MIN_Y>MIN_Y&&xc+MIN_X<MAX_X&&yc+MIN_Y<MAX_Y)
	{
		putpixel(xc+MIN_X,yc+MIN_Y,Current_Color);
		putpixel(xc+1+MIN_X,yc+1+MIN_Y,Current_Color);
		putpixel(xc+MIN_X,yc+1+MIN_Y,Current_Color);
		putpixel(xc+1+MIN_X,yc+MIN_Y,Current_Color);
	}
	else
		outtextxy(150,316 ,"sorry! pixel is outside the boundary");
}
void trans3d()
{
	 char *xs,*ys,*zs,*chs,*txs,*tys,*tzs,*sxs,*sys,*szs,*angs;
	 float a[10][10],d[10][10]/*,c[9][9]*/;
	 int tx,ty,tz,sx,sy,sz,ch;
	 int x,y,z,ang,i,j;
	 float th;
	 setfillstyle(SOLID_FILL,WHITE);
	 bar(MIN_X+2,MIN_Y+2,MAX_X-2,MAX_Y-2);
	 xs=readline("Enter x cordinate:(0-525) ");
	 x=atoi(xs);
	 ys=readline("Enter y cordinate:(0-290) ");
	 y=atoi(ys);
	 zs=readline("Enter z cordinate:(0-300) ");
	 z=atoi(zs);
	 putpixel(x+MIN_X,y+MIN_Y,Current_Color);
	 putpixel(x+1+MIN_X,y+1+MIN_Y,Current_Color);
	 putpixel(x+1+MIN_X,y+MIN_Y,Current_Color);
	 putpixel(x+MIN_X,y+1+MIN_Y,Current_Color);
	 d[0][0]=x,d[0][1]=y,d[0][2]=z,d[0][3]=1;
	 chs=readline("1.TRANS 2.SCALING 3.Z-rotate 4.Y-rotate 5.X-rotate");
	 ch=atoi(chs);
	 switch(ch)
	 {
		case 1: txs=readline("Enter tx: ");
				  tx=atoi(txs);
				  tys=readline("Enter ty ");
				  ty=atoi(tys);
				  tzs=readline("Enter tz: ");
				  tz=atoi(tzs);
				  a[0][0]=a[1][1]=a[2][2]=a[3][3]=1;
				  a[3][0]=tx;a[3][1]=ty;a[3][2]=tz;
				  a[0][1]=a[0][2]=a[0][3]=a[1][0]=a[1][2]=a[1][3]=a[2][0]=a[2][1]=a[2][3]=0;
				  mul(d,a); break;
		case 2: sxs=readline("Enter sx: ");
				  sx=atoi(sxs);
				  sys=readline("Enter sy ");
				  sy=atoi(sys);
				  szs=readline("Enter sz: ");
				  sz=atoi(szs);
				  for(i=0;i<4;i++)
					 for(j=0;j<4;j++)
						a[i][j]=0;
					a[0][0]=sx;a[1][1]=sy;
					a[2][2]=sz;a[3][3]=1;
				  mul(d,a); break;
		case 3: angs=readline("enter rotatinal angle about z");
				  ang=atoi(angs);
				  th=(3.142*ang)/180;
				  a[0][0]=a[1][1]=cos(th);
				  a[0][2]=a[1][2]=a[0][3]=a[1][3]=a[2][1]=a[2][0]=0;
				  a[3][0]=a[3][1]=a[3][2]=a[2][3]=0;
				  a[3][3]=a[2][2]=1;
				  a[0][1]=sin(th);a[1][0]=-sin(th);
				  mul(d,a);break;
		case 4: angs=readline("enter rotatinal angle about y");
				  ang=atoi(angs);
				  th=(3.142*ang)/180;
				  a[1][1]=a[3][3]=1;
				  a[0][1]=a[0][3]=a[1][0]=a[1][2]=a[1][3]=a[2][1]=0;
				  a[2][3]=a[3][0]=a[3][1]=a[3][2]=0;
				  a[0][0]=a[2][2]=cos(th);
				  a[2][0]=sin(th);
				  a[0][2]=-sin(th);
				  mul(d,a); break;
		case 5: angs=readline("enter rotatinal angle about x");
				  ang=atoi(angs);
				  th=(3.142*ang)/180;
				  a[0][0]=a[3][3]=1;
				  a[1][1]=a[2][2]=cos(th);
				  a[1][2]=sin(th);a[2][1]=-sin(th);
				  a[0][1]=a[0][2]=a[0][3]=a[1][0]=a[1][3]=a[2][0]=0;
				  a[2][3]=a[3][0]=a[3][1]=a[3][2]=0;
				  mul(d,a);
				  break;
		default:exit(0);
	 }
}
void bezier(int x1,int y1)
{
	 int x4,y4,x3,y3,x2,y2;
	 double a,b,t;
	 x4=mousex;y4=mousey;
	 x2=87,y2=47;
	 x3=632,y3=427;
	 setcolor(Current_Color);
	 hidemouse();
	 for(t=0;t<1;t+=0.001)
	 {
		a=(1-t)*(1-t)*(1-t)*x1+3*t*(1-t)*(1-t)*x2+3*t*t*(1-t)*x3+t*t*t*x4;
		b=(1-t)*(1-t)*(1-t)*y1+3*t*(1-t)*(1-t)*y2+3*t*t*(1-t)*y3+t*t*t*y4;
		if((a>(MIN_X + 2)&&a<(MAX_X - 2))&&(b>(MIN_Y + 2)&&b<(MAX_Y - 2)))
			line((int)a,(int)b,(int)a,(int)b);
	 }
	 showmouse();
}
void draw_bezier()
{
	 int x,y;
	 showmouse();
	 getxy();
	 x=mousex;y=mousey;
	 disp_cordn();
	 setwritemode(XOR_PUT);
	 if(LeftPressed)
	 {
		while(LeftPressed)
		{
			getxy();disp_cordn();
			if(prevx==mousex&&prevy==mousey)
				continue;
			//putback();
			showmouse();
			getxy();
			hidemouse();
			setfillstyle(SOLID_FILL,WHITE);
			bar(MIN_X+2,MIN_Y+2,MAX_X-2,MAX_Y-2); //new sheet
			bezier(x,y);
			setwritemode(COPY_PUT);
			restrictmouse(3,3,637,477);
		}
		showmouse();
	 }
	 showmouse();
}
void translate()
{
	 char *name;
	 int tx,ty;
	 name=readline("Enter the translation factor tx: ");
	 tx=atoi(name);
	 name=readline("Enter the translation factor ty: ");
	 ty=atoi(name);
	 setfillstyle(SOLID_FILL,WHITE);
	 getch();
	 bar(MIN_X+2,MIN_Y+2,MAX_X-2,MAX_Y-2);  //clears the work area and removes old rect to make sure translated rect can be displayed
	 if(xr2+tx >MAX_X)
		xr2=MAX_X;
	 else if(xr2+tx < MIN_Y)
		xr2=MIN_Y;
	 else
		xr2+=tx;
	 if(yr2+ty >MAX_Y)
		yr2=MAX_Y;
	 else if(yr2+ty <MIN_Y)
		yr2=MIN_Y;
	 else
		yr2+=ty;
	 if(xr1+tx <MIN_X)
		xr1=MIN_X;
	 else if(xr1+tx >MAX_X)
		xr1=MAX_X;
	 else
		xr1=xr1+tx;
	 if(yr1+ty<MIN_Y)
		yr1=MIN_Y;
	 else if(yr1+ty >MAX_Y)
		yr1=MAX_Y;
	 else
		yr1+=ty;
	 setcolor(Current_Color);
	 rectangle(xr1,yr1,xr2,yr2);
}
void scale()
{
	 char *name;
	 int sx,sy;
	 name=readline("Enter the scaling factor sx: ");
	 sx=atoi(name);
	 name=readline("Enter the scaling factor sy: ");
	 sy=atoi(name);
	 setfillstyle(SOLID_FILL,WHITE);
	 getch();
	 bar(MIN_X+2,MIN_Y+2,MAX_X-2,MAX_Y-2);  //clears the work area and removes old rect to make sure translated rect can be displayed
	 if(xr2*sx >MAX_X)
		xr2=MAX_X;
	 else if(xr2*sx < MIN_Y)
		xr2=MIN_Y;
	 else
		xr2*=sx;
	 if(yr2*sy >MAX_Y)
		yr2=MAX_Y;
	 else if(yr2*sy <MIN_Y)
		yr2=MIN_Y;
	 else
		yr2*=sy;
	 if(xr1*sx <MIN_X)
		xr1=MIN_X;
	 else if(xr1*sx >MAX_X)
		xr1=MAX_X;
	 else
		xr1*=sx;
	 if(yr1*sy<MIN_Y)
		yr1=MIN_Y;
	 else if(yr1*sy >MAX_Y)
		yr1=MAX_Y;
	 else
		yr1*=sy;
	setcolor(Current_Color);
	rectangle(xr1,yr1,xr2,yr2);
}
void rotate()
{
	char *name;
	float ang,th;
	int dx,dy;
	int x[5],y[5];
	name=readline("Enter the rotation angle: ");
	ang=atof(name);
	setfillstyle(SOLID_FILL,WHITE);
	getch();
	bar(MIN_X+2,MIN_Y+2,MAX_X-2,MAX_Y-2);  //clears the work area and removes old rect to make sure translated rect can be displayed
	th= (3.1416 * ang)/180.0;
	dx=xr2-xr1, dy=yr2-yr1;
	x[1]= xr1  	,	y[1]=yr1;
	x[2]= dx*cos(th) +xr1  ,	 y[2]= dx*sin(th)+yr1;
	x[3]= -dy*sin(th) +xr1  ,     y[3]= dy*cos(th)+yr1;
	x[4]= dx*cos(th) - dy*sin(th) + xr1;
	y[4]= dx*sin(th) + dy*cos(th) + yr1;
	setcolor(Current_Color);
	line(x[1],y[1],x[2],y[2]);
	line(x[2],y[2],x[4],y[4]);
	line(x[4],y[4],x[3],y[3]);
	line(x[3],y[3],x[1],y[1]);
	xr1=x[1];
	yr1=y[1];
	xr2=x[4];
	yr2=y[4];
}
void trans2d()
{
	char *name,msg[15];
	int choice;
	setcolor(Current_Color);
	setfillstyle(SOLID_FILL,WHITE);
	bar(MIN_X+2,MIN_Y+2,MAX_X-2,MAX_Y-2);  //clears the work area and removes old rect to make sure translated rect can be displayed
	show_status("Draw rectangle for 2D transformation");
	drawrectangle();
	hidemouse();
	sprintf(msg,"(%d,%d)",xr1,yr1);
	outtextxy(xr1,yr1-textheight("1"),msg);
	sprintf(msg,"(%d,%d)",xr2,yr2);
	outtextxy(xr2,yr2+textheight("1"),msg);
	showmouse();
	name=readline("1.Translation 2.Scaling 3.Rotation");
	choice=atoi(name);
	switch(choice)
	{
		 case 1: translate();
			 break;
		 case 2: scale();
			 break;
		 case 3: rotate();
			 break;
		 default: break;
	}
	sprintf(msg,"(%d,%d)",xr1,yr1);
	outtextxy(xr1,yr1-textheight("1"),msg);
	sprintf(msg,"(%d,%d)",xr2,yr2);
	outtextxy(xr2,yr2+textheight("1"),msg);
}
void polygonfill()
{
	 int x,y,ymax=0,ymin=480,a[20][20],xi[20];
	 int i,j,k,n,temp,dx,dy;
	 float slope[20];
	 char *read,msg[30];
	 read=readline("Enter the number of vertices: ");
	 n=atoi(read);
	 hidemouse();
	 for(i=0;i<n;i++)
	 {
		sprintf(msg,"x[%d]<1-520>: ",i+1);
		read=readline(msg);
		x=atoi(read)+MIN_X;
		sprintf(msg,"y[%d]<1-326>: ",i+1);
		read=readline(msg);
		y=atoi(read)+MIN_Y;
		if(y>ymax)
			 ymax=y;
		if(y<ymin)
			 ymin=y;
		a[i][0] = x;
		a[i][1] = y;
	 }
	 a[n][0] = a[0][0];
	 a[n][1] = a[0][1];
	 for(i=0;i<n;i++)
	 {
		line(a[i][0],a[i][1],a[i+1][0],a[i+1][1]);
	 }
	 for(i=0;i<n;i++)
	 {
		dy = a[i+1][1] - a[i][1];
		dx = a[i+1][0] - a[i][0];
		if(dy == 0)		slope[i] = 1.0;
		if(dx == 0)		slope[i] = 0.0;
		if((dy != 0) && (dx != 0))
		{
			slope[i] = (float)dx/dy;
		}
	}
	for(y=0;y<480;y++)
	{
	  k=0;
	  for(i=0;i<n;i++)
	  {
		 if(((a[i][1] <= y)&&(a[i+1][1] > y))||((a[i][1] > y)&&(a[i+1][1] <= y)))
		 {
			xi[k] = (int)(a[i][0] + slope[i] * (y-a[i][1]));
			k++;
		 }
	  }
	  for(j=0;j<k-1;j++)
	  for(i=0;i<k-1;i++)
	  {
			if(xi[i] > xi[i+1])
			{
				temp = xi[i];
				xi[i] = xi[i+1];
				xi[i+1] = temp;
			}
		}
		setcolor(5);
		for(i=0;i<k;i+=2)
		{
			line(xi[i],y,xi[i+1]+1,y);
			delay(50);
		}
	}
	showmouse();
}
void initialize()
{
	int x_min=20,y_min=50, x_max=45,y_max=75;
	int butt=0; // button 1
	settextstyle(2,HORIZ_DIR,5);
	init_button(butt,x_min,y_min,x_max,y_max,"DRAW A CIRCLE");
	draw_button(butt);  //MIGHT BE BUTT++
	setcolor(BLACK);
	circle(32,62,7);
	butt++; y_min = y_max+10; y_max = y_min+25;
	init_button(butt,x_min,y_min,x_max,y_max,"LINE AROUND A CIRCLE");
	draw_button(butt);
	setcolor(BLACK);
	circle(32,97,5);
	line(22,98,33,87);
	butt++; y_min = y_max+10; y_max = y_min+25;
	init_button(butt,x_min,y_min,x_max,y_max,"DRAWS SPIRAL");
	draw_button(butt);
	setcolor(BLACK);
	settextstyle(2,HORIZ_DIR,4);
	outtextxy(22,126,"spir");
	butt++; y_min = y_max+10; y_max = y_min+25;
	init_button(butt,x_min,y_min,x_max,y_max,"2D TRANSFORMATION");
	draw_button(butt);
	setcolor(BLACK);
	settextstyle(2,HORIZ_DIR,5);
	outtextxy(25,160,"2D");
	butt++; y_min = y_max+10; y_max = y_min+25;
	init_button(butt,x_min,y_min,x_max,y_max,"HERMITE CURVE");
	draw_button(butt);
	setcolor(BLACK);
	settextstyle(2,HORIZ_DIR,7);
	outtextxy(28,190,"H");
	butt++; y_min = y_max+10; y_max = y_min+25;
	init_button(butt,x_min,y_min,x_max,y_max,"LINE CLIPPING");
	draw_button(butt);
	setcolor(BLACK);
	settextstyle(2,HORIZ_DIR,4);
	outtextxy(22,230,"clip");
	butt++; y_min = y_max+10; y_max = y_min+25;
	init_button(butt,x_min,y_min,x_max,y_max,"ROTATE A WHEEL");
	draw_button(butt);
	setcolor(BLACK);
	circle(32,272,7);
	line(25,272,39,272);
	line(32,265,32,279);
	butt++;
	x_min=55; x_max=80; y_min=50; y_max=75;
	init_button(butt,x_min,y_min,x_max,y_max,"DRAW A LINE");
	draw_button(butt);
	setcolor(BLACK);
	line(60,70,75,55);
	butt++; y_min = y_max+10; y_max = y_min+25;
	init_button(butt,x_min,y_min,x_max,y_max,"DRAW AN ELLIPSE");
	draw_button(butt);
	setcolor(BLACK);
	settextstyle(2,HORIZ_DIR,4);
	outtextxy(57,90,"elli");
	butt++; y_min = y_max+10; y_max = y_min+25;
	init_button(butt,x_min,y_min,x_max,y_max,"SEGMENTATION");
	draw_button(butt);
	setcolor(BLACK);
	settextstyle(2,HORIZ_DIR,4);
	outtextxy(59,125,"Seg");
	butt++; y_min = y_max+10; y_max = y_min+25;
	init_button(butt,x_min,y_min,x_max,y_max,"3D TRANSFORMATION");
	draw_button(butt);
	setcolor(BLACK);
	settextstyle(2,HORIZ_DIR,5);
	outtextxy(60,160,"3D");
	butt++; y_min = y_max+10; y_max = y_min+25;
	init_button(butt,x_min,y_min,x_max,y_max,"BEZIER CURVE");
	draw_button(butt);
	setcolor(BLACK);
	settextstyle(2,HORIZ_DIR,7);
	outtextxy(63,190,"B");
	butt++; y_min = y_max+10; y_max = y_min+25;
	init_button(butt,x_min,y_min,x_max,y_max,"POLYGON FILLING");
	draw_button(butt);
	setcolor(BLACK);
	settextstyle(2,HORIZ_DIR,4);
	outtextxy(57,230,"poly");
	//butt++; y_min = y_max+10; y_max = y_min+25;
	//init_button(butt,x_min,y_min,x_max,y_max,"ERASE");
	//draw_button(butt);
	//setfillstyle(SOLID_FILL,WHITE);
	//bar(60,268,75,277);
	setfillstyle(SOLID_FILL,Current_Color);
	setcolor(BLACK);
}
void operation()
{
		switch(Current_Button)
		{
			case 0:bcircle();
					 break;
			case 1:rotate_line();
					 break;
			case 2:drawspiral();
					 break;
			case 3:trans2d();
					 break;
			case 4:hermite_curve();
					 break;
			case 5:clip();
					 break;
			case 6:drawwheel();
					 break;
			case 7:draw_line();
					 break;
			case 8:draw_ellipse();
					 break;
			case 9:segmentation();
					 break;
			case 10:trans3d();
					  break;
			case 11:draw_bezier();
					  break;
			case 12:polygonfill();
					  break;
		}//switch ends
}
void color_index(int x1,int y1,int x2,int y2,int color)
{
	setfillstyle(SOLID_FILL,color);
	icon(x1-1,y1-1,x2+1,y2+1,0);
	bar(x1,y1,x2,y2);
	colorbutton[color].xmin=x1;
	colorbutton[color].ymin=y1;
	colorbutton[color].xmax=x2;
	colorbutton[color].ymax=y2;
	colorbutton[color].color=color;
}
int select_color()
{
	int i;
	getxy();
	for(i=0;i<16;i++)
	{
		if(check_mouse_pos(colorbutton[i].xmin,colorbutton[i].ymin,colorbutton[i].xmax,colorbutton[i].ymax))
		{
			Current_Color = colorbutton[i].color;
			setfillstyle(SOLID_FILL,Current_Color);
			bar(38,348,62,357);
			return 1;
		}
	}
	return 0;
}
void menu_icon()
{
	setcolor(WHITE);
	icon(10,10,34,23,0);// "clear" button
	settextstyle(2,HORIZ_DIR,4);
	outtextxy(13,11,"CLR");
	settextstyle(2,HORIZ_DIR,7);
}
void screen_design()
{
	int i,a,b,dist,width,color;
	setcolor(WHITE);
	rectangle(3,3,637,477);//sets entire screen
	setfillstyle(SOLID_FILL,LIGHTGRAY);
	floodfill(10,20,WHITE);//fills the region above WHITE line with BLUE
	setcolor(BLUE);
	line(4,25,636,25);
	setfillstyle(SOLID_FILL,BLUE);
	floodfill(10,20,WHITE);//fills the region above WHITE line with BLUE
	settextstyle(8,HORIZ_DIR,1);
	setcolor(WHITE);
	outtextxy(285,1,"PAINT");
	settextstyle(3,HORIZ_DIR,1);
	setcolor(WHITE);
	outtextxy(625,1,"X");
	icon(MIN_X,MIN_Y,MAX_X,MAX_Y,0);  //creates the working window
	icon(MIN_X+1,MIN_Y+1,MAX_X+1,MAX_Y+1,1);
	setfillstyle(SOLID_FILL,WHITE);
	bar(MIN_X+2,MIN_Y+2,MAX_X-2,MAX_Y-2);//sets the working window to WHITE color
	icon(10,MIN_Y,90,MAX_Y,0);  //creates the icon window
	icon(11,MIN_Y+1,91,MAX_Y+1,1);
	icon(622,5,635,23,0);// exit icon
	icon(10,452,500,472,1); //help window
	icon(35,345,65,360,1); //creates Current_Color window
	setfillstyle(SOLID_FILL,Current_Color);
	bar(38,348,62,357); //shows the Current_Color
	icon(MIN_X,390,265,440,0);// color panel
	icon(500,380,MAX_X,430,1); //info about the designer
	setcolor(BLACK);
	settextstyle(2,HORIZ_DIR,5);
	color=getcolor();
	setcolor(4);
	settextstyle(2,HORIZ_DIR,4);
	outtextxy(508,380,"Designed by: ");
	setcolor(1);
	outtextxy(550,390,"SHRUTHY S");
	outtextxy(550,403,"SHRUTHI G");
	outtextxy(550,416,"SHRIDEVI N D");
	setcolor(color);
	icon(530,440,MAX_X-30,472,1); //To display x and y co-ordinates
	// To set the color panel with all 16colors
	a=105; dist=5;width=15;
	for(i=0;i<8;i++)
	{
		color_index(a,395,a+width,410,i);
		a=a+width+dist;
	}
	a=105; dist=5;width=15;
	for(i=8;i<16;i++)
	{
		color_index(a,420,a+width,435,i);//sets color to color panel
		a=a+width+dist;
	}
	menu_icon();//To create menu - new,clear
}
int exit_button()
{
	if(check_mouse_pos(622,5,635,23) && LeftPressed)
		return 1;
	return 0;
}
void main()
{
	int gd=DETECT,gm,mstatus;
	clrscr();
	initgraph(&gd,&gm,"..\\BGI\\");
	screen_design();
	initmouse();
	//To set cursor position to the centre of the working window
	startmouse((MIN_X+MAX_X)/2,(MIN_Y+MAX_Y)/2);
	showmouse();//shows cursor
	initialize();
	while(1)
	{
		getxy();
		disp_cordn();
		if(LeftPressed)
		{
		 if(check_if_button_pressed()>=0) ;
		 else if(check_if_menu()) ;
		 else if(select_color())
			clear_status();
		 else if(check_mouse_pos(MIN_X,MIN_Y,MAX_X,MAX_Y))
		 {
			clear_status();
			operation();
		 }
		 else if(exit_button())
				exit(0);
		}
		else (check_if_button_pressed());
	}
}