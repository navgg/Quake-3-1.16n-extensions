//(c) 2019 Navigator (322)
//bright models

xm_fb1
{
	{
		map textures/sfx/snow.jpg
		blendFunc add
		rgbGen entity
		tcMod scale 0.3 0.3
		tcMod scroll 0 -0.02
	}
	{
		map gfx/damage/shadow.tga
		blendFunc blend
		rgbGen entity
		tcMod scale 0.35 0.35
	}
}

xm_fb2
{
	{
		map textures/sfx/shadow.jpg
		blendFunc GL_ONE GL_ZERO
		rgbGen entity
	}
}

xm_fb3
{
	{
		map $whiteimage
		blendFunc GL_ONE GL_ZERO
		rgbGen entity
	}
}

//transparent weapon

xm_whiteShader
{
	{
		map $whiteimage
		blendfunc add
		rgbGen entity
	}
}

//weather

xm_snowflake2
{
	cull none
	entityMergable
	{
		map gfx/misc/snowflake2.tga
		blendFunc blend
		rgbGen vertex
	}
}

//crosshairs

gfx/2d/xm_crosshair
{
	nopicmip
	{
		map gfx/2d/crosshair.tga          
		blendFunc blend               
	    rgbGen vertex
	}
}

gfx/2d/xm_crosshaira
{
	nopicmip
	{
		map gfx/2d/crosshaira.tga
		blendFunc blend               
	    rgbGen vertex
	}
}

gfx/2d/xm_crosshairb
{
	nopicmip
	{
		map gfx/2d/crosshairb.tga
		blendFunc blend
	    rgbGen vertex
	}
}

gfx/2d/xm_crosshairc
{
	nopicmip
	{
		map gfx/2d/crosshairc.tga
		blendFunc blend
	    rgbGen vertex
	}
}

gfx/2d/xm_crosshaird
{
	nopicmip
	{
		map gfx/2d/crosshaird.tga
		blendFunc blend
	    rgbGen vertex
	}
}

gfx/2d/xm_crosshaire
{
	nopicmip
	{
		map gfx/2d/crosshaire.tga
		blendFunc blend
	    rgbGen vertex
	}
}

gfx/2d/xm_crosshairf
{
	nopicmip
	{
		map gfx/2d/crosshairf.tga
		blendFunc blend
	    rgbGen vertex
	}
}

gfx/2d/xm_crosshairg
{
	nopicmip
	{
		map gfx/2d/crosshairg.tga
		blendFunc blend
	    rgbGen vertex
	}
}

gfx/2d/xm_crosshairh
{
	nopicmip
	{
		map gfx/2d/crosshairh.tga
		blendFunc blend
	    rgbGen vertex
	}
}

gfx/2d/xm_crosshairi
{
	nopicmip
	{
		map gfx/2d/crosshairi.tga
		blendFunc blend
	    rgbGen vertex
	}
}

gfx/2d/xm_crosshairj
{
	nopicmip
	{
		map gfx/2d/crosshairj.tga
		blendFunc blend
		rgbGen vertex
	}
}

gfx/2d/xm_crosshairk
{
	nopicmip
	{
		map gfx/2d/crosshairk.tga
		blendFunc blend
	    rgbGen vertex
	}
}
