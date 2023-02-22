#include "pch.h"
#include "Detours/detours.h"
#include "zSTRING.h"

#include <ddraw.h>
#include <d3d.h>

bool NewBinkSetVolume = true;
DWORD BinkOpenWaveOut;
DWORD BinkSetSoundSystem;
DWORD BinkSetSoundOnOff;
DWORD BinkSetVolume;
DWORD BinkOpen;
DWORD BinkDoFrame;
DWORD BinkNextFrame;
DWORD BinkWait;
DWORD BinkPause;
DWORD BinkClose;
DWORD BinkGoto;
DWORD BinkCopyToBuffer;
DWORD BinkSetFrameRate;
DWORD BinkSetSimulate;

inline DWORD UTIL_power_of_2(DWORD input)
{
	DWORD value = 1;
	while(value < input) value <<= 1;
	return value;
}

struct BinkVideo
{
	BinkVideo(void* vid) : vid(vid) {}

	void* vid = nullptr;

	unsigned char* textureData = nullptr;
	LPDIRECTDRAWSURFACE7 texture = nullptr;
	DWORD width = 0;
	DWORD height = 0;
	bool useBGRA = false;

	float scaleTU = 1.f;
	float scaleTV = 1.f;

	float globalVolume = 1.f;
	float videoVolume = 1.f;
	bool updateVolume = true;
	bool scaleVideo = true;
};

float BinkPlayerReadGlobalVolume(DWORD zCOption)
{
#if defined(BUILD_GOTHIC_1_08k)
#if defined(BUILD_1_12F)
    return reinterpret_cast<float(__thiscall*)(DWORD, DWORD, DWORD, float)>(0x465CD0)(zCOption, 0x8ADE00, 0x8ADF08, 1.f);
#else
    return reinterpret_cast<float(__thiscall*)(DWORD, DWORD, DWORD, float)>(0x45E370)(zCOption, 0x869190, 0x869270, 1.f);
#endif
#else
    return reinterpret_cast<float(__thiscall*)(DWORD, DWORD, DWORD, float)>(0x463A60)(zCOption, 0x8CD380, 0x8CD49C, 1.f);
#endif
}

bool BinkPlayerReadScaleVideos( DWORD zCOption )
{
#if defined(BUILD_GOTHIC_1_08k)
#if defined(BUILD_1_12F)
    return reinterpret_cast<int(__thiscall*)(DWORD, DWORD, DWORD, int)>(GothicMemoryLocations::zCOption::ReadBool)(zCOption, 0x8AE058, 0x874228, 1);
#else
    return reinterpret_cast<int(__thiscall*)(DWORD, DWORD, DWORD, int)>(GothicMemoryLocations::zCOption::ReadBool)(zCOption, 0x869388, 0x82E6E4, 1);
#endif
#else
    return reinterpret_cast<int(__thiscall*)(DWORD, DWORD, DWORD, int)>(GothicMemoryLocations::zCOption::ReadBool)(zCOption, 0x8CD5F0, 0x8925D8, 1);
#endif
}

int __fastcall BinkPlayerPlayHandleEvents(DWORD BinkPlayer)
{
	reinterpret_cast<void(__cdecl*)(void)>(GothicMemoryLocations::GlobalObjects::sysEvents)();

	BinkVideo* video = *reinterpret_cast<BinkVideo**>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_VideoHandle);
	if(*reinterpret_cast<int*>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_DisallowInputHandling)) return 0;
	if(!video) return 0;
	if(!(*reinterpret_cast<int*>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_DoHandleEvents))) return 1;

	DWORD zInput = *reinterpret_cast<DWORD*>(GothicMemoryLocations::GlobalObjects::zCInput);
	WORD key = reinterpret_cast<WORD(__thiscall*)(DWORD, int, int)>(*reinterpret_cast<DWORD*>
        (*reinterpret_cast<DWORD*>(zInput) + GothicMemoryLocations::zCInput::GetKey_Offset))(zInput, 0, 0);
	reinterpret_cast<void(__thiscall*)(DWORD)>(*reinterpret_cast<DWORD*>
        (*reinterpret_cast<DWORD*>(zInput) + GothicMemoryLocations::zCInput::ProcessInputEvents_Offset))(zInput);
	switch(key)
	{
		case 0x01: // DIK_ESCAPE
			reinterpret_cast<void(__thiscall*)(DWORD)>(*reinterpret_cast<DWORD*>
                (*reinterpret_cast<DWORD*>(BinkPlayer) + GothicMemoryLocations::zCBinkPlayer::Stop_Offset))(BinkPlayer);
			break;
		case 0x39: // DIK_SPACE
        {
			if(*reinterpret_cast<int*>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_IsPaused))
			{
				reinterpret_cast<void(__stdcall*)(void*, int)>(BinkPause)(video->vid, 0);
				*reinterpret_cast<int*>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_IsPaused) = 0;
			}
			else
			{
				reinterpret_cast<void(__stdcall*)(void*, int)>(BinkPause)(video->vid, 1);
				*reinterpret_cast<int*>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_IsPaused) = 1;
			}
		}
		break;
		case 0x10: // DIK_Q
        {
			if(*reinterpret_cast<int*>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_SoundOn))
			{
				video->globalVolume = 0.f;
				video->updateVolume = true;
				*reinterpret_cast<int*>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_SoundOn) = 0;
			}
			else
			{
                video->globalVolume = BinkPlayerReadGlobalVolume(*reinterpret_cast<DWORD*>(GothicMemoryLocations::GlobalObjects::zCOption));
				video->updateVolume = true;
				*reinterpret_cast<int*>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_SoundOn) = 1;
			}
		}
		break;
		case 0xC8: // DIK_UP
        {
			video->videoVolume = std::min<float>(1.0f, video->videoVolume + 0.05f);
			video->updateVolume = true;
		}
		break;
		case 0xD0: // DIK_DOWN
        {
			video->videoVolume = std::max<float>(0.0f, video->videoVolume - 0.05f);
			video->updateVolume = true;
		}
		break;
	}
	return 1;
}

float __fastcall BinkPlayerSetSoundVolume(DWORD BinkPlayer, DWORD _EDX, float volume)
{
	BinkVideo* video = *reinterpret_cast<BinkVideo**>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_VideoHandle);
	video->videoVolume = std::min<float>(1.0f, std::max<float>(video->videoVolume, 0.0f));
	video->updateVolume = true;
	return 1;
}

int __fastcall BinkPlayerToggleSound(DWORD BinkPlayer)
{
	BinkVideo* video = *reinterpret_cast<BinkVideo**>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_VideoHandle);
	if(*reinterpret_cast<int*>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_SoundOn))
	{
		video->globalVolume = 0.f;
		video->updateVolume = true;
		*reinterpret_cast<int*>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_SoundOn) = 0;
	}
	else
	{
        video->globalVolume = BinkPlayerReadGlobalVolume(*reinterpret_cast<DWORD*>(GothicMemoryLocations::GlobalObjects::zCOption));
		video->updateVolume = true;
		*reinterpret_cast<int*>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_SoundOn) = 1;
	}
	return 1;
}

int __fastcall BinkPlayerPause(DWORD BinkPlayer)
{
	BinkVideo* video = *reinterpret_cast<BinkVideo**>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_VideoHandle);
	reinterpret_cast<void(__stdcall*)(void*, int)>(BinkPause)(video->vid, 1);
	return 1;
}

int __fastcall BinkPlayerUnpause(DWORD BinkPlayer)
{
	BinkVideo* video = *reinterpret_cast<BinkVideo**>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_VideoHandle);
	reinterpret_cast<void(__stdcall*)(void*, int)>(BinkPause)(video->vid, 0);
	return 1;
}

int __fastcall BinkPlayerIsPlaying(DWORD BinkPlayer)
{
	BinkVideo* video = *reinterpret_cast<BinkVideo**>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_VideoHandle);
	if(video &&
		(*reinterpret_cast<int*>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_IsPlaying)) &&
		((*reinterpret_cast<int*>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_IsLooping)) ||
		*reinterpret_cast<DWORD*>(reinterpret_cast<DWORD>(video->vid) + 0x08) > *reinterpret_cast<DWORD*>(reinterpret_cast<DWORD>(video->vid) + 0x0C)))
		return 1;

	return 0;
}

int __fastcall BinkPlayerPlayGotoNextFrame(DWORD BinkPlayer)
{
	BinkVideo* video = *reinterpret_cast<BinkVideo**>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_VideoHandle);
	reinterpret_cast<void(__stdcall*)(void*)>(BinkNextFrame)(video->vid);
	return 1;
}

int __fastcall BinkPlayerPlayWaitNextFrame(DWORD BinkPlayer)
{
	BinkVideo* video = *reinterpret_cast<BinkVideo**>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_VideoHandle);
	while(BinkPlayerIsPlaying(BinkPlayer) && reinterpret_cast<int(__stdcall*)(void*)>(BinkWait)(video->vid))
    {
        BinkPlayerPlayHandleEvents(BinkPlayer);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	return 1;
}

int __fastcall BinkPlayerPlayDoFrame(DWORD BinkPlayer)
{
	BinkVideo* video = *reinterpret_cast<BinkVideo**>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_VideoHandle);
	if(video->updateVolume)
    {
		float volume = video->globalVolume * video->videoVolume;
		//if(NewBinkSetVolume) reinterpret_cast<void(__stdcall*)(void*, int, DWORD)>(BinkSetVolume)(video->vid, 0, static_cast<DWORD>(volume * 65536.f));
		//else reinterpret_cast<void(__stdcall*)(void*, DWORD)>(BinkSetVolume)(video->vid, static_cast<DWORD>(volume * 65536.f));
		video->updateVolume = false;
	}
	reinterpret_cast<void(__stdcall*)(void*)>(BinkDoFrame)(video->vid);
	return 1;
}

int __fastcall BinkPlayerPlayFrame(DWORD BinkPlayer)
{
	BinkVideo* video = *reinterpret_cast<BinkVideo**>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_VideoHandle);
	if(BinkPlayerIsPlaying(BinkPlayer))
	{
        BinkPlayerPlayHandleEvents(BinkPlayer);
		if(BinkPlayerIsPlaying(BinkPlayer))
		{
            BinkPlayerPlayDoFrame(BinkPlayer);
			{
				DWORD vidWidth = *reinterpret_cast<DWORD*>(reinterpret_cast<DWORD>(video->vid) + 0x00);
				DWORD vidHeight = *reinterpret_cast<DWORD*>(reinterpret_cast<DWORD>(video->vid) + 0x04);
				if(!video->texture || video->width != vidWidth || video->height != vidHeight)
				{
					video->useBGRA = false;
					video->width = vidWidth;
					video->height = vidHeight;
					if(video->texture)
					{
						video->texture->Release();
						video->texture = nullptr;
					}
					video->textureData = new unsigned char[vidWidth * vidHeight * 4];

					DDSURFACEDESC2 ddsd;
					ZeroMemory(&ddsd, sizeof(ddsd));
					ddsd.dwSize = sizeof(ddsd);
					ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
					ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY;
					ddsd.ddsCaps.dwCaps2 = DDSCAPS2_HINTDYNAMIC;
					ddsd.dwWidth = UTIL_power_of_2(vidWidth);
					ddsd.dwHeight = UTIL_power_of_2(vidHeight);
					ddsd.ddpfPixelFormat.dwSize = sizeof(ddsd.ddpfPixelFormat);
					ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
					ddsd.ddpfPixelFormat.dwRGBBitCount = 32;
					ddsd.ddpfPixelFormat.dwRBitMask = 0x000000FF;
					ddsd.ddpfPixelFormat.dwGBitMask = 0x0000FF00;
					ddsd.ddpfPixelFormat.dwBBitMask = 0x00FF0000;
					ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xFF000000;
					HRESULT hr = (*reinterpret_cast<LPDIRECTDRAW7*>(GothicMemoryLocations::zCRndD3D::DDRAW7))->CreateSurface(&ddsd, &video->texture, nullptr);
					if(FAILED(hr))
					{
						video->useBGRA = true;
						ddsd.ddpfPixelFormat.dwRBitMask = 0x00FF0000;
						ddsd.ddpfPixelFormat.dwGBitMask = 0x0000FF00;
						ddsd.ddpfPixelFormat.dwBBitMask = 0x000000FF;
						hr = (*reinterpret_cast<LPDIRECTDRAW7*>(GothicMemoryLocations::zCRndD3D::DDRAW7))->CreateSurface(&ddsd, &video->texture, nullptr);
						if(FAILED(hr))
						{
							*reinterpret_cast<int*>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_IsPlaying) = 0;
							return 0;
						}
					}

					video->scaleTU = 1.0f / ddsd.dwWidth;
					video->scaleTV = 1.0f / ddsd.dwHeight;
				}

				int srcPitch = vidWidth * 4;
				reinterpret_cast<void(__stdcall*)(void*, void*, int, DWORD, DWORD, DWORD, DWORD)>(BinkCopyToBuffer)
					(video->vid, video->textureData, vidWidth * 4, vidHeight, 0, 0, (video->useBGRA ? 3 : 4));
				if(video->texture->IsLost() == DDERR_SURFACELOST)
					video->texture->Restore();

				DDSURFACEDESC2 ddsd;
				ZeroMemory(&ddsd, sizeof(ddsd));
				ddsd.dwSize = sizeof(ddsd);
				HRESULT hr = video->texture->Lock(nullptr, &ddsd, DDLOCK_NOSYSLOCK | DDLOCK_WAIT | DDLOCK_WRITEONLY, nullptr);
				if(FAILED(hr))
				{
					*reinterpret_cast<int*>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_IsPlaying) = 0;
					return 0;
				}

				if(ddsd.lPitch == srcPitch)
					memcpy(ddsd.lpSurface, video->textureData, srcPitch * vidHeight);
				else
				{
					unsigned char* dstData = reinterpret_cast<unsigned char*>(ddsd.lpSurface);
					unsigned char* srcData = video->textureData;
					for(DWORD h = 0; h < vidHeight; ++h)
					{
						memcpy(dstData, srcData, srcPitch);
						dstData += ddsd.lPitch;
						srcData += srcPitch;
					}
				}
				video->texture->Unlock(nullptr);

				DWORD zrenderer = *reinterpret_cast<DWORD*>(GothicMemoryLocations::GlobalObjects::zRenderer);
				int oldZWrite = reinterpret_cast<int(__thiscall*)(DWORD)>(*reinterpret_cast<DWORD*>
                    (*reinterpret_cast<DWORD*>(zrenderer) + GothicMemoryLocations::zCRndD3D::GetZBufferWriteEnabled_Offset))(zrenderer);
				reinterpret_cast<void(__thiscall*)(DWORD, int)>(*reinterpret_cast<DWORD*>
                    (*reinterpret_cast<DWORD*>(zrenderer) + GothicMemoryLocations::zCRndD3D::SetZBufferWriteEnabled_Offset))(zrenderer, 0); // No depth-writes
				int oldZCompare = reinterpret_cast<int(__thiscall*)(DWORD)>(*reinterpret_cast<DWORD*>
                    (*reinterpret_cast<DWORD*>(zrenderer) + GothicMemoryLocations::zCRndD3D::GetZBufferCompare_Offset))(zrenderer);
				int newZCompare = 0; // Compare always
				reinterpret_cast<void(__thiscall*)(DWORD, int&)>(*reinterpret_cast<DWORD*>
                    (*reinterpret_cast<DWORD*>(zrenderer) + GothicMemoryLocations::zCRndD3D::SetZBufferCompare_Offset))(zrenderer, newZCompare);
				int oldAlphaFunc = reinterpret_cast<int(__thiscall*)(DWORD)>(*reinterpret_cast<DWORD*>
                    (*reinterpret_cast<DWORD*>(zrenderer) + GothicMemoryLocations::zCRndD3D::GetAlphaBlendFunc_Offset))(zrenderer);
				int oldFilter = reinterpret_cast<int(__thiscall*)(DWORD)>(*reinterpret_cast<DWORD*>
                    (*reinterpret_cast<DWORD*>(zrenderer) + GothicMemoryLocations::zCRndD3D::GetBilerpFilterEnabled_Offset))(zrenderer);
				reinterpret_cast<void(__thiscall*)(DWORD, int)>(*reinterpret_cast<DWORD*>
                    (*reinterpret_cast<DWORD*>(zrenderer) + GothicMemoryLocations::zCRndD3D::SetBilerpFilterEnabled_Offset))(zrenderer, video->scaleVideo ? 1 : 0); // Bilinear filter
				int oldFog = reinterpret_cast<int(__thiscall*)(DWORD)>(*reinterpret_cast<DWORD*>
                    (*reinterpret_cast<DWORD*>(zrenderer) + GothicMemoryLocations::zCRndD3D::GetFog_Offset))(zrenderer);
				reinterpret_cast<void(__thiscall*)(DWORD, int)>(*reinterpret_cast<DWORD*>
                    (*reinterpret_cast<DWORD*>(zrenderer) + GothicMemoryLocations::zCRndD3D::SetFog_Offset))(zrenderer, 0); // No fog

                DWORD SetTextureStageState = *reinterpret_cast<DWORD*>(*reinterpret_cast<DWORD*>(zrenderer) + GothicMemoryLocations::zCRndD3D::SetTextureStageState_Offset);
				// Disable alpha blending
				reinterpret_cast<void(__thiscall*)(DWORD, int, int)>(GothicMemoryLocations::zCRndD3D::XD3D_SetRenderState)(zrenderer, 26, 0);
				reinterpret_cast<void(__thiscall*)(DWORD, int, int)>(GothicMemoryLocations::zCRndD3D::XD3D_SetRenderState)(zrenderer, 27, 0);
				reinterpret_cast<void(__thiscall*)(DWORD, int, int)>(GothicMemoryLocations::zCRndD3D::XD3D_SetRenderState)(zrenderer, 15, 0);
				// Disable clipping
				reinterpret_cast<void(__thiscall*)(DWORD, int, int)>(GothicMemoryLocations::zCRndD3D::XD3D_SetRenderState)(zrenderer, 136, 0);
				// Disable culling
				reinterpret_cast<void(__thiscall*)(DWORD, int, int)>(GothicMemoryLocations::zCRndD3D::XD3D_SetRenderState)(zrenderer, 22, 1);
				// Set texture clamping
				reinterpret_cast<void(__thiscall*)(DWORD, int, int, int)>(SetTextureStageState)(zrenderer, 0, 12, 3);
				reinterpret_cast<void(__thiscall*)(DWORD, int, int, int)>(SetTextureStageState)(zrenderer, 0, 13, 3);
				// 0 stage AlphaOp modulate
				reinterpret_cast<void(__thiscall*)(DWORD, int, int, int)>(SetTextureStageState)(zrenderer, 0, 3, 3);
				// 1 stage AlphaOp disable
				reinterpret_cast<void(__thiscall*)(DWORD, int, int, int)>(SetTextureStageState)(zrenderer, 1, 3, 0);
				// 0 stage ColorOp modulate
				reinterpret_cast<void(__thiscall*)(DWORD, int, int, int)>(SetTextureStageState)(zrenderer, 0, 0, 3);
				// 1 stage ColorOp disable
				reinterpret_cast<void(__thiscall*)(DWORD, int, int, int)>(SetTextureStageState)(zrenderer, 1, 0, 0);
				// 0 stage AlphaArg1/2 texure/diffuse
				reinterpret_cast<void(__thiscall*)(DWORD, int, int, int)>(SetTextureStageState)(zrenderer, 0, 4, 3);
				reinterpret_cast<void(__thiscall*)(DWORD, int, int, int)>(SetTextureStageState)(zrenderer, 0, 5, 1);
				// 0 stage ColorArg1/2 texure/diffuse
				reinterpret_cast<void(__thiscall*)(DWORD, int, int, int)>(SetTextureStageState)(zrenderer, 0, 1, 3);
				reinterpret_cast<void(__thiscall*)(DWORD, int, int, int)>(SetTextureStageState)(zrenderer, 0, 2, 1);
				// 0 stage TextureTransformFlags disable
				reinterpret_cast<void(__thiscall*)(DWORD, int, int, int)>(SetTextureStageState)(zrenderer, 0, 23, 0);
				// 0 stage TexCoordIndex 0
				reinterpret_cast<void(__thiscall*)(DWORD, int, int, int)>(SetTextureStageState)(zrenderer, 0, 10, 0);

				// Set fullscreen viewport
				int gWidth = *reinterpret_cast<int*>(zrenderer + GothicMemoryLocations::zCRndD3D::Offset_Width);
				int gHeight = *reinterpret_cast<int*>(zrenderer + GothicMemoryLocations::zCRndD3D::Offset_Height);
				reinterpret_cast<void(__thiscall*)(DWORD, int, int, int, int)>(*reinterpret_cast<DWORD*>
                    (*reinterpret_cast<DWORD*>(zrenderer) + GothicMemoryLocations::zCRndD3D::SetViewport_Offset))(zrenderer, 0, 0, gWidth, gHeight);

				// Set video texture
				reinterpret_cast<void(__thiscall*)(DWORD, int, LPDIRECTDRAWSURFACE7)>(GothicMemoryLocations::zCRndD3D::XD3D_SetTexture)(zrenderer, 0, video->texture);

				{
					struct D3DTLVERTEX
					{
						float sx;
						float sy;
						float sz;
						float rhw;
						DWORD color;
						DWORD specular;
						float tu;
						float tv;
					};
					D3DTLVERTEX vertices[8];

					float scale = std::min<float>(static_cast<float>(gWidth) / static_cast<float>(video->width), static_cast<float>(gHeight) / static_cast<float>(video->height));
					int dstW = std::min<int>(static_cast<int>(video->width * scale), gWidth);
					int dstH = std::min<int>(static_cast<int>(video->height * scale), gHeight);
					int dstX = std::max<int>((gWidth / 2) - (dstW / 2), 0);
					int dstY = std::max<int>((gHeight / 2) - (dstH / 2), 0);
					if(!video->scaleVideo)
					{
						dstX = (gWidth / 2) - (video->width / 2);
						dstY = (gHeight / 2) - (video->height / 2);
						dstW = video->width;
						dstH = video->height;
					}

					float minx = static_cast<float>(dstX);
					float miny = static_cast<float>(dstY);
					float maxx = static_cast<float>(dstW) + minx;
					float maxy = static_cast<float>(dstH) + miny;

					float minu = 0.f;
					float maxu = static_cast<float>(video->width) * video->scaleTU;
					float minv = 0.f;
					float maxv = static_cast<float>(video->height) * video->scaleTV;

					vertices[0].sx = 0.f;
					vertices[0].sy = 0.f;
					vertices[0].sz = 0.f;
					vertices[0].rhw = 1.f;
					vertices[0].color = 0x00000000;
					vertices[0].specular = 0x00000000;
					vertices[0].tu = 0.f;
					vertices[0].tv = 0.f;

					vertices[1].sx = static_cast<float>(gWidth);
					vertices[1].sy = 0.f;
					vertices[1].sz = 0.f;
					vertices[1].rhw = 1.f;
					vertices[1].color = 0x00000000;
					vertices[1].specular = 0x00000000;
					vertices[1].tu = 0.f;
					vertices[1].tv = 0.f;

					vertices[2].sx = static_cast<float>(gWidth);
					vertices[2].sy = static_cast<float>(gHeight);
					vertices[2].sz = 0.f;
					vertices[2].rhw = 1.f;
					vertices[2].color = 0x00000000;
					vertices[2].specular = 0x00000000;
					vertices[2].tu = 0.f;
					vertices[2].tv = 0.f;

					vertices[3].sx = 0.f;
					vertices[3].sy = static_cast<float>(gHeight);
					vertices[3].sz = 0.f;
					vertices[3].rhw = 1.f;
					vertices[3].color = 0x00000000;
					vertices[3].specular = 0x00000000;
					vertices[3].tu = 0.f;
					vertices[3].tv = 0.f;

					vertices[4].sx = minx;
					vertices[4].sy = miny;
					vertices[4].sz = 0.f;
					vertices[4].rhw = 1.f;
					vertices[4].color = 0xFFFFFFFF;
					vertices[4].specular = 0xFFFFFFFF;
					vertices[4].tu = minu;
					vertices[4].tv = minv;

					vertices[5].sx = maxx;
					vertices[5].sy = miny;
					vertices[5].sz = 0.f;
					vertices[5].rhw = 1.f;
					vertices[5].color = 0xFFFFFFFF;
					vertices[5].specular = 0xFFFFFFFF;
					vertices[5].tu = maxu;
					vertices[5].tv = minv;

					vertices[6].sx = maxx;
					vertices[6].sy = maxy;
					vertices[6].sz = 0.f;
					vertices[6].rhw = 1.f;
					vertices[6].color = 0xFFFFFFFF;
					vertices[6].specular = 0xFFFFFFFF;
					vertices[6].tu = maxu;
					vertices[6].tv = maxv;

					vertices[7].sx = minx;
					vertices[7].sy = maxy;
					vertices[7].sz = 0.f;
					vertices[7].rhw = 1.f;
					vertices[7].color = 0xFFFFFFFF;
					vertices[7].specular = 0xFFFFFFFF;
					vertices[7].tu = minu;
					vertices[7].tv = maxv;

					LPDIRECT3DDEVICE7 d3d7Device = *reinterpret_cast<LPDIRECT3DDEVICE7*>(GothicMemoryLocations::zCRndD3D::D3DDevice7);
					d3d7Device->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX, reinterpret_cast<LPVOID>(vertices + 0), 4, 0);
					d3d7Device->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX, reinterpret_cast<LPVOID>(vertices + 4), 4, 0);
				}

				reinterpret_cast<void(__thiscall*)(DWORD, int)>(*reinterpret_cast<DWORD*>
                    (*reinterpret_cast<DWORD*>(zrenderer) + GothicMemoryLocations::zCRndD3D::SetFog_Offset))(zrenderer, oldFog);
				reinterpret_cast<void(__thiscall*)(DWORD, int)>(*reinterpret_cast<DWORD*>
                    (*reinterpret_cast<DWORD*>(zrenderer) + GothicMemoryLocations::zCRndD3D::SetBilerpFilterEnabled_Offset))(zrenderer, oldFilter);
				reinterpret_cast<void(__thiscall*)(DWORD, int&)>(*reinterpret_cast<DWORD*>
                    (*reinterpret_cast<DWORD*>(zrenderer) + GothicMemoryLocations::zCRndD3D::SetAlphaBlendFunc_Offset))(zrenderer, oldAlphaFunc);
				reinterpret_cast<void(__thiscall*)(DWORD, int&)>(*reinterpret_cast<DWORD*>
                    (*reinterpret_cast<DWORD*>(zrenderer) + GothicMemoryLocations::zCRndD3D::SetZBufferCompare_Offset))(zrenderer, oldZCompare);
				reinterpret_cast<void(__thiscall*)(DWORD, int)>(*reinterpret_cast<DWORD*>
                    (*reinterpret_cast<DWORD*>(zrenderer) + GothicMemoryLocations::zCRndD3D::SetZBufferWriteEnabled_Offset))(zrenderer, oldZWrite);
				reinterpret_cast<void(__thiscall*)(DWORD, int, void*, void*)>(*reinterpret_cast<DWORD*>
                    (*reinterpret_cast<DWORD*>(zrenderer) + GothicMemoryLocations::zCRndD3D::Vid_Blit_Offset))(zrenderer, 0, nullptr, nullptr);
			}
            BinkPlayerPlayGotoNextFrame(BinkPlayer);
            BinkPlayerPlayWaitNextFrame(BinkPlayer);
		}
	}
	return 1;
}

int __fastcall BinkPlayerPlayInit(DWORD BinkPlayer, DWORD _EDX, int frame)
{
	BinkVideo* video = *reinterpret_cast<BinkVideo**>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_VideoHandle);
	if(!video)
		return 0;

    if(!reinterpret_cast<int(__thiscall*)(DWORD, int)>(GothicMemoryLocations::zCBinkPlayer::PlayInit)(BinkPlayer, frame))
    {
        *reinterpret_cast<int*>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_IsPlaying) = 0;
        return 1;
    }

	if(frame > 0)
		reinterpret_cast<void(__stdcall*)(void*, int, int)>(BinkGoto)(video->vid, frame, 0);

	return 1;
}

int __fastcall BinkPlayerPlayDeinit(DWORD BinkPlayer)
{
	DWORD BackView = *reinterpret_cast<DWORD*>(BinkPlayer + 0x5C);
    if(BackView)
    {
#if defined(BUILD_GOTHIC_1_08k)
        reinterpret_cast<void(__thiscall*)(DWORD, int)>(*reinterpret_cast<DWORD*>(*reinterpret_cast<DWORD*>(BackView) + 0x20))(BackView, 1);
#else
        reinterpret_cast<void(__thiscall*)(DWORD, int)>(*reinterpret_cast<DWORD*>(*reinterpret_cast<DWORD*>(BackView) + 0x24))(BackView, 1);
#endif
    }

	return reinterpret_cast<int(__thiscall*)(DWORD)>(GothicMemoryLocations::zCBinkPlayer::PlayDeinit)(BinkPlayer);
}

int __fastcall BinkPlayerOpenVideo(DWORD BinkPlayer, DWORD _EDX, zSTRING videoName)
{
    DWORD zCOption = *reinterpret_cast<DWORD*>(GothicMemoryLocations::GlobalObjects::zCOption);
#if defined(BUILD_GOTHIC_1_08k) && !defined(BUILD_1_12F)
	zSTRING& directoryRoot = reinterpret_cast<zSTRING&(__thiscall*)(DWORD, int)>(GothicMemoryLocations::zCOption::GetDirectory)(zCOption, 23);
#else
	zSTRING& directoryRoot = reinterpret_cast<zSTRING&(__thiscall*)(DWORD, int)>(GothicMemoryLocations::zCOption::GetDirectory)(zCOption, 24);
#endif
	std::string pathToVideo = std::string(directoryRoot.ToChar(), directoryRoot.Length()) +
		std::string(videoName.ToChar(), videoName.Length());
	if(pathToVideo.find(".BIK") == std::string::npos && pathToVideo.find(".BK2") == std::string::npos)
		pathToVideo.append(".BIK");

	reinterpret_cast<void(__stdcall*)(DWORD, DWORD)>(BinkSetSoundSystem)(BinkOpenWaveOut, 0);
	void* videoHandle = reinterpret_cast<void*(__stdcall*)(const char*, DWORD)>(BinkOpen)(pathToVideo.c_str(), 0);
	if(videoHandle)
	{
		reinterpret_cast<void(__stdcall*)(void*, int)>(BinkSetSoundOnOff)(videoHandle, 1);
		//if(NewBinkSetVolume) reinterpret_cast<void(__stdcall*)(void*, int, DWORD)>(BinkSetVolume)(videoHandle, 0, 65536);
		//else reinterpret_cast<void(__stdcall*)(void*, DWORD)>(BinkSetVolume)(videoHandle, 65536);
		*reinterpret_cast<BinkVideo**>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_VideoHandle) = new BinkVideo(videoHandle);
	}

	BinkVideo* video = *reinterpret_cast<BinkVideo**>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_VideoHandle);
	if(video)
	{
        video->globalVolume = BinkPlayerReadGlobalVolume(zCOption);
        video->scaleVideo = BinkPlayerReadScaleVideos(zCOption);

		// We are passing directly zSTRING so the memory will be deleted inside this function
		reinterpret_cast<int(__thiscall*)(DWORD, zSTRING)>(GothicMemoryLocations::zCBinkPlayer::OpenVideo)(BinkPlayer, videoName);
		return 1;
	}

	videoName.Delete();
	return 0;
}

int __fastcall BinkPlayerCloseVideo(DWORD BinkPlayer)
{
	BinkVideo* video = *reinterpret_cast<BinkVideo**>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_VideoHandle);
	if(!video)
		return 0;

	delete[] video->textureData;
	if(video->texture)
	{
		video->texture->Release();
		video->texture = nullptr;
	}
	reinterpret_cast<void(__stdcall*)(void*)>(BinkClose)(video->vid);

	delete video;
	*reinterpret_cast<DWORD*>(BinkPlayer + GothicMemoryLocations::zCBinkPlayer::Offset_VideoHandle) = 0;
	return reinterpret_cast<int(__thiscall*)(DWORD)>(GothicMemoryLocations::zCBinkPlayer::CloseVideo)(BinkPlayer);
}

void RegisterBinkPlayerHooks()
{
    HMODULE BinkWDLL;
	if((BinkWDLL = LoadLibraryA("Bink2W32.dll")) != nullptr || (BinkWDLL = GetModuleHandleA("BinkW32.dll")) != nullptr)
	{
		NewBinkSetVolume = true;
		BinkOpenWaveOut = reinterpret_cast<DWORD>(GetProcAddress(BinkWDLL, "_BinkOpenWaveOut@4"));
		BinkSetSoundSystem = reinterpret_cast<DWORD>(GetProcAddress(BinkWDLL, "_BinkSetSoundSystem@8"));
		BinkSetSoundOnOff = reinterpret_cast<DWORD>(GetProcAddress(BinkWDLL, "_BinkSetSoundOnOff@8"));
		BinkSetVolume = reinterpret_cast<DWORD>(GetProcAddress(BinkWDLL, "_BinkSetVolume@12"));
		if(!BinkSetVolume)
		{
			BinkSetVolume = reinterpret_cast<DWORD>(GetProcAddress(BinkWDLL, "_BinkSetVolume@8"));
#if defined(BUILD_GOTHIC_1_08k)
#if !defined(BUILD_1_12F)
			if(*reinterpret_cast<BYTE*>(0x43A942) != 0xE9)
				NewBinkSetVolume = false;
#else
            NewBinkSetVolume = false;
#endif
#endif
		}
		BinkOpen = reinterpret_cast<DWORD>(GetProcAddress(BinkWDLL, "_BinkOpen@8"));
		BinkDoFrame = reinterpret_cast<DWORD>(GetProcAddress(BinkWDLL, "_BinkDoFrame@4"));
		BinkNextFrame = reinterpret_cast<DWORD>(GetProcAddress(BinkWDLL, "_BinkNextFrame@4"));
		BinkWait = reinterpret_cast<DWORD>(GetProcAddress(BinkWDLL, "_BinkWait@4"));
		BinkPause = reinterpret_cast<DWORD>(GetProcAddress(BinkWDLL, "_BinkPause@8"));
		BinkClose = reinterpret_cast<DWORD>(GetProcAddress(BinkWDLL, "_BinkClose@4"));
		BinkGoto = reinterpret_cast<DWORD>(GetProcAddress(BinkWDLL, "_BinkGoto@12"));
		BinkCopyToBuffer = reinterpret_cast<DWORD>(GetProcAddress(BinkWDLL, "_BinkCopyToBuffer@28"));
		BinkSetFrameRate = reinterpret_cast<DWORD>(GetProcAddress(BinkWDLL, "_BinkSetFrameRate@8"));
		BinkSetSimulate = reinterpret_cast<DWORD>(GetProcAddress(BinkWDLL, "_BinkSetSimulate@4"));
	}
    char* binkPlayerVtable[5];

    DWORD playHandleEvents = reinterpret_cast<DWORD>(&BinkPlayerPlayHandleEvents);
    memcpy( binkPlayerVtable, &playHandleEvents, 4 );
    PatchAddr( GothicMemoryLocations::zCBinkPlayer::PlayHandleEvents_Vtable, binkPlayerVtable );
    PatchJMP( GothicMemoryLocations::zCBinkPlayer::PlayHandleEvents_Func, reinterpret_cast<DWORD>(&BinkPlayerPlayHandleEvents) );

    DWORD setSoundVolume = reinterpret_cast<DWORD>(&BinkPlayerSetSoundVolume);
    memcpy( binkPlayerVtable, &setSoundVolume, 4 );
    PatchAddr( GothicMemoryLocations::zCBinkPlayer::SetSoundVolume_Vtable, binkPlayerVtable );
    PatchJMP( GothicMemoryLocations::zCBinkPlayer::SetSoundVolume_Func, reinterpret_cast<DWORD>(&BinkPlayerSetSoundVolume) );

    DWORD toggleSound = reinterpret_cast<DWORD>(&BinkPlayerToggleSound);
    memcpy( binkPlayerVtable, &toggleSound, 4 );
    PatchAddr( GothicMemoryLocations::zCBinkPlayer::ToggleSound_Vtable, binkPlayerVtable );
    PatchJMP( GothicMemoryLocations::zCBinkPlayer::ToggleSound_Func, reinterpret_cast<DWORD>(&BinkPlayerToggleSound) );

    DWORD pause = reinterpret_cast<DWORD>(&BinkPlayerPause);
    memcpy( binkPlayerVtable, &pause, 4 );
    PatchAddr( GothicMemoryLocations::zCBinkPlayer::Pause_Vtable, binkPlayerVtable );
    PatchJMP( GothicMemoryLocations::zCBinkPlayer::Pause_Func, reinterpret_cast<DWORD>(&BinkPlayerPause) );

    DWORD unpause = reinterpret_cast<DWORD>(&BinkPlayerUnpause);
    memcpy( binkPlayerVtable, &unpause, 4 );
    PatchAddr( GothicMemoryLocations::zCBinkPlayer::Unpause_Vtable, binkPlayerVtable );
    PatchJMP( GothicMemoryLocations::zCBinkPlayer::Unpause_Func, reinterpret_cast<DWORD>(&BinkPlayerUnpause) );

    DWORD isPlaying = reinterpret_cast<DWORD>(&BinkPlayerIsPlaying);
    memcpy( binkPlayerVtable, &isPlaying, 4 );
    PatchAddr( GothicMemoryLocations::zCBinkPlayer::IsPlaying_Vtable, binkPlayerVtable );
    PatchJMP( GothicMemoryLocations::zCBinkPlayer::IsPlaying_Func, reinterpret_cast<DWORD>(&BinkPlayerIsPlaying) );
    
    DWORD playGotoNextFrame = reinterpret_cast<DWORD>(&BinkPlayerPlayGotoNextFrame);
    memcpy( binkPlayerVtable, &playGotoNextFrame, 4 );
    PatchAddr( GothicMemoryLocations::zCBinkPlayer::PlayGotoNextFrame_Vtable, binkPlayerVtable );
    PatchJMP( GothicMemoryLocations::zCBinkPlayer::PlayGotoNextFrame_Func, reinterpret_cast<DWORD>(&BinkPlayerPlayGotoNextFrame) );

    DWORD playWaitNextFrame = reinterpret_cast<DWORD>(&BinkPlayerPlayWaitNextFrame);
    memcpy( binkPlayerVtable, &playWaitNextFrame, 4 );
    PatchAddr( GothicMemoryLocations::zCBinkPlayer::PlayWaitNextFrame_Vtable, binkPlayerVtable );
    PatchJMP( GothicMemoryLocations::zCBinkPlayer::PlayWaitNextFrame_Func, reinterpret_cast<DWORD>(&BinkPlayerPlayWaitNextFrame) );

    DWORD playFrame = reinterpret_cast<DWORD>(&BinkPlayerPlayFrame);
    memcpy( binkPlayerVtable, &playFrame, 4 );
    PatchAddr( GothicMemoryLocations::zCBinkPlayer::PlayFrame_Vtable, binkPlayerVtable );
    PatchJMP( GothicMemoryLocations::zCBinkPlayer::PlayFrame_Func, reinterpret_cast<DWORD>(&BinkPlayerPlayFrame) );

    DWORD playInit = reinterpret_cast<DWORD>(&BinkPlayerPlayInit);
    memcpy( binkPlayerVtable, &playInit, 4 );
    PatchAddr( GothicMemoryLocations::zCBinkPlayer::PlayInit_Vtable, binkPlayerVtable );
    PatchJMP( GothicMemoryLocations::zCBinkPlayer::PlayInit_Func, reinterpret_cast<DWORD>(&BinkPlayerPlayInit) );

    DWORD playDeinit = reinterpret_cast<DWORD>(&BinkPlayerPlayDeinit);
    memcpy( binkPlayerVtable, &playDeinit, 4 );
    PatchAddr( GothicMemoryLocations::zCBinkPlayer::PlayDeinit_Vtable, binkPlayerVtable );
    PatchJMP( GothicMemoryLocations::zCBinkPlayer::PlayDeinit_Func, reinterpret_cast<DWORD>(&BinkPlayerPlayDeinit) );

    DWORD openVideo = reinterpret_cast<DWORD>(&BinkPlayerOpenVideo);
    memcpy( binkPlayerVtable, &openVideo, 4 );
    PatchAddr( GothicMemoryLocations::zCBinkPlayer::OpenVideo_Vtable, binkPlayerVtable );
    PatchJMP( GothicMemoryLocations::zCBinkPlayer::OpenVideo_Func, reinterpret_cast<DWORD>(&BinkPlayerOpenVideo) );

    DWORD closeVideo = reinterpret_cast<DWORD>(&BinkPlayerCloseVideo);
    memcpy( binkPlayerVtable, &closeVideo, 4 );
    PatchAddr( GothicMemoryLocations::zCBinkPlayer::CloseVideo_Vtable, binkPlayerVtable );
    PatchJMP( GothicMemoryLocations::zCBinkPlayer::CloseVideo_Func, reinterpret_cast<DWORD>(&BinkPlayerCloseVideo) );
}
