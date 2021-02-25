// anmobj.cpp: implementation of the anmobj class.
//程序设计：唐明理	2005.2
//E_mail cqtml@163.com
//////////////////////////////////////////////////////////////////////
#include "anmobj.h"
#include "EyeGame.h"
//#include "baiscobj.h"

//////////////////////////////////////////////////////////////////////
int dongzuo[7]={0,2,2,7,12,14,14};

//extern baiscobj* m_baiscobj;

//////////////////////////////////////////////////////////////////////
anmobj::anmobj()
{
	model[0] = NULL;	model[1] = NULL;
	getobj();
	for (int i = 0; i < RNSHU; i++)
	{
		man[i].dz = 1;
	}
}
anmobj::~anmobj()
{}

void anmobj::setman(int i, float y)
{	if(model[0]==NULL) return;

   //srand(timeGetTime());
	counter[i]++;
	if(counter[i]>1)
	{	counter[i]=0;
		frame[i]++;
		if(man[i].dz==1)
		{man[i].qd[0]-=.3f;
		 if(man[i].qd[0]-man[i].zd[0]<0)
		 {man[i].dz=dongzuo[rand()%7];
		  frame[i]=anim[0][man[i].dz].start;
		 }
		}
		if(frame[i]>anim[0][man[i].dz].end)
		{	if(man[i].dz!=1) man[i].dz=dongzuo[rand()%7];
			frame[i]=anim[0][man[i].dz].start;
		}
	}
    glPushAttrib(GL_CURRENT_BIT);
	glPushMatrix();
	//glTranslatef(man[i].qd[0], y, -man[i].qd[1])；
	glTranslatef(0,0,0);
	glRotatef(-90,1,0,0);
	glScaled(.03f,.03f,.03f);

	glEnable(GL_TEXTURE_2D);
	if(model[0]!=NULL)
	{	//texture[0]->ChangeSettings(GL_LINEAR,GL_LINEAR_MIPMAP_LINEAR,GL_CLAMP,GL_CLAMP,0);
		md2_drawModel (model[0],(int)frame[i],0,0);
	}
	if(model[1]!=NULL /*&& man[i].dz<size[1]-1*/)
	{	//texture[1]->ChangeSettings(GL_LINEAR,GL_LINEAR_MIPMAP_LINEAR,GL_CLAMP,GL_CLAMP,0);
		md2_drawModel (model[1],(int)frame[i],0,0);
	}
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glPopAttrib();
}
void anmobj::getobj()
{	
	anim[0]=anim[1]=NULL;
	if(model[0]!=NULL) {delete[] model[0];model[0]=NULL;}
	if(model[1]!=NULL) {delete[] model[1];model[1]=NULL;}
	model[0]   = md2_readModel("tris.md2");
	//texture[0] = new Texture("tris.PCX");
	model[1]   = md2_readModel("weapon.md2");
	//texture[1] = new Texture("weapon.pcx");
	

	if(model[0]!=NULL) 
	{
		size[0] = md2_getAnimationCount(model[0]);
		animations(model[0], 0);//
	}
	if (model[1] != NULL)
	{
		size[1] = md2_getAnimationCount(model[1]);
		animations(model[1], 1);//
	}

	 
	 for(int i=0;i<RNSHU;i++)
		{counter[i]=0;
		 frame[i]=(float)(40+rand()%5);//
		}
	}
animation* anmobj::animations (md2_model_t* model,int p)
{
	
	if (size[p] == 0) return 0;//
	if(anim[p]!=NULL) delete[] anim[p];//
	anim[p] = new animation[size[p]];//
	for (int i=0; i<size[p]; i++)//
	{
		strcpy (anim[p][i].name,md2_getAnimationName(model,i));//
		std::cout << p << " " << i << " " << anim[p][i].name << std::endl;
		md2_getAnimationFrames(model,i,&anim[p][i].start,&anim[0][i].end);//
	}

	return 0;//
}
