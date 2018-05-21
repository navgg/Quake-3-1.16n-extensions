freezeShader
{
	deformvertexes wave 100 sin 3 0 0 0
	{
		map textures/effects/envmap.tga
		blendfunc gl_one gl_one
		tcgen environment
	}
}
freezeMarkShader
{
	nopicmip
	polygonoffset
	{
		clampmap gfx/damage/freeze_stain.tga
		blendfunc gl_src_alpha gl_one_minus_src_alpha
		rgbgen identitylighting
		alphagen vertex
	}
}
