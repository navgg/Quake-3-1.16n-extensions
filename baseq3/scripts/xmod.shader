xm_bright
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

xm_whiteShader
{
	{
		map $whiteimage
		blendfunc add
		rgbGen entity
	}
}