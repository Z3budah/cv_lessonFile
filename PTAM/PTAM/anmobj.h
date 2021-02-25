// anmobj.h: interface for the anmobj class.
//
//////////////////////////////////////////////////////////////////////
#include"md2.h"
//#include"../include/texture.h"
#include "GLWindow2.h"
#include "OpenGL.h"


#if !defined(AFX_ANMOBJ_H__46249C44_7C55_4407_AAE7_790A324FB6DE__INCLUDED_)
#define AFX_ANMOBJ_H__46249C44_7C55_4407_AAE7_790A324FB6DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct animation
{	char name[16];
	int	 start;		
	int	 end;		
};

struct MAN
 {	float qd[3];
	float zd[3];
	int dz;	
 }; 

#define RNSHU 2
////////////////////////////////////////////
class anmobj
{
public:
	anmobj();
	virtual ~anmobj();
public:
	md2_model_t*	model[2];
	//Texture*		texture[2]; 
	animation*		anim[2];
	int				size[2];
	float			frame[RNSHU];
	MAN				man[RNSHU];
	int				counter[RNSHU];
	void			getobj();
	animation*		animations (md2_model_t* model,int p);
	void			setman(int i, float y);
///////////////////////////////////////////////////////////////////
	bool			manmove(int i);
	int				re[RNSHU];
///////////////////////////////////////////////////////////////////////
};

#endif // !defined(AFX_ANMOBJ_H__46249C44_7C55_4407_AAE7_790A324FB6DE__INCLUDED_)
