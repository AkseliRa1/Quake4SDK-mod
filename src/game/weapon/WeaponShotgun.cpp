#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"
#include "../Weapon.h"

const int SHOTGUN_MOD_AMMO = BIT(0);

class rvWeaponShotgun : public rvWeapon
{
public:
	CLASS_PROTOTYPE(rvWeaponShotgun);

	rvWeaponShotgun(void);

	virtual void Spawn(void);
	void Save(idSaveGame *savefile) const;
	void Restore(idRestoreGame *savefile);
	void PreSave(void);
	void PostSave(void);

protected:
	int hitscans;

	bool UpdateFlashlight(void);
	void Flashlight(bool on);

private:
	stateResult_t State_Idle(const stateParms_t &parms);
	stateResult_t State_Fire(const stateParms_t &parms);
	stateResult_t State_Reload(const stateParms_t &parms);
	stateResult_t State_Flashlight(const stateParms_t &parms);

	CLASS_STATES_PROTOTYPE(rvWeaponShotgun);
};

CLASS_DECLARATION(rvWeapon, rvWeaponShotgun)
END_CLASS

/*
================
rvWeaponShotgun::rvWeaponShotgun
================
*/
rvWeaponShotgun::rvWeaponShotgun(void)
{
}
	
/*
================
rvWeaponShotgun::Spawn
================
*/
void rvWeaponShotgun::Spawn(void)
{
	hitscans = spawnArgs.GetFloat("hitscans");

	SetState("Raise", 0);

	// Override flashlight properties because weapon_shotgun.def is missing them
	const idDeclEntityDef* mgDef = gameLocal.FindEntityDef("weapon_machinegun", false);
	if (mgDef)
	{
		const idDict& mgDict = mgDef->dict;
		renderLight_t& light = lights[WPLIGHT_FLASHLIGHT];
		
		light.shader = declManager->FindMaterial(mgDict.GetString("mtr_flashlightShader", "lights/muzzleflash"), false);
		idVec4 color;
		mgDict.GetVec4("flashlightColor", "0 0 0 0", color);
		light.shaderParms[SHADERPARM_RED] = color[0];
		light.shaderParms[SHADERPARM_GREEN] = color[1];
		light.shaderParms[SHADERPARM_BLUE] = color[2];
		
		light.lightRadius[0] = light.lightRadius[1] = light.lightRadius[2] = (float)mgDict.GetInt("flashlightRadius", "400");
		
		light.pointLight = mgDict.GetBool("flashlightPointLight", "1");
		if (!light.pointLight)
		{
			light.target = mgDict.GetVector("flashlightTarget");
			light.up = mgDict.GetVector("flashlightUp");
			light.right = mgDict.GetVector("flashlightRight");
			light.end = light.target;
		}

		mgDict.GetVector("flashlightViewOffset", "0 0 0", flashlightViewOffset);

		// Copy overridden properties to the world flashlight
		lights[WPLIGHT_FLASHLIGHT_WORLD] = light;
		lights[WPLIGHT_FLASHLIGHT_WORLD].allowLightInViewID = 0;
		lights[WPLIGHT_FLASHLIGHT_WORLD].suppressLightInViewID = owner->entityNumber + 1;
		lights[WPLIGHT_FLASHLIGHT_WORLD].lightId = WPLIGHT_FLASHLIGHT_WORLD * 100 + owner->entityNumber;
	}

	Flashlight(owner->IsFlashlightOn());
}

/*
================
rvWeaponShotgun::Save
================
*/
void rvWeaponShotgun::Save(idSaveGame *savefile) const
{
}

/*
================
rvWeaponShotgun::Restore
================
*/
void rvWeaponShotgun::Restore(idRestoreGame *savefile)
{
	hitscans = spawnArgs.GetFloat("hitscans");
}

/*
================
rvWeaponShotgun::PreSave
================
*/
void rvWeaponShotgun::PreSave(void)
{
}

/*
================
rvWeaponShotgun::PostSave
================
*/
void rvWeaponShotgun::PostSave(void)
{
}

/*
===============================================================================

	States

===============================================================================
*/

CLASS_STATES_DECLARATION(rvWeaponShotgun)
STATE("Idle", rvWeaponShotgun::State_Idle)
STATE("Fire", rvWeaponShotgun::State_Fire)
STATE("Reload", rvWeaponShotgun::State_Reload)
STATE("Flashlight", rvWeaponShotgun::State_Flashlight)
END_CLASS_STATES

/*
================
rvWeaponShotgun::State_Idle
================
*/
stateResult_t rvWeaponShotgun::State_Idle(const stateParms_t &parms)
{
	enum
	{
		STAGE_INIT,
		STAGE_WAIT,
	};
	switch (parms.stage)
	{
	case STAGE_INIT:
		if (!AmmoAvailable())
		{
			SetStatus(WP_OUTOFAMMO);
		}
		else
		{
			SetStatus(WP_READY);
		}

		PlayCycle(ANIMCHANNEL_ALL, "idle", parms.blendFrames);
		return SRESULT_STAGE(STAGE_WAIT);

	case STAGE_WAIT:
		if (wsfl.lowerWeapon)
		{
			SetState("Lower", 4);
			return SRESULT_DONE;
		}
		if (UpdateFlashlight())
		{
			return SRESULT_DONE;
		}
		if (!clipSize)
		{
			if (gameLocal.time > nextAttackTime && wsfl.attack && AmmoAvailable())
			{
				SetState("Fire", 0);
				return SRESULT_DONE;
			}
		}
		else
		{
			if (gameLocal.time > nextAttackTime && wsfl.attack && AmmoInClip())
			{
				SetState("Fire", 0);
				return SRESULT_DONE;
			}
			if (wsfl.attack && AutoReload() && !AmmoInClip() && AmmoAvailable())
			{
				SetState("Reload", 4);
				return SRESULT_DONE;
			}
			if (wsfl.netReload || (wsfl.reload && AmmoInClip() < ClipSize() && AmmoAvailable() > AmmoInClip()))
			{
				SetState("Reload", 4);
				return SRESULT_DONE;
			}
		}
		return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponShotgun::State_Fire
================
*/
stateResult_t rvWeaponShotgun::State_Fire(const stateParms_t &parms)
{
	enum
	{
		STAGE_INIT,
		STAGE_WAIT,
	};
	switch (parms.stage)
	{
	case STAGE_INIT:
		nextAttackTime = gameLocal.time + (fireRate * owner->PowerUpModifier(PMOD_FIRERATE));
		Attack(false, hitscans, spread, 0, 1.0f);
		PlayAnim(ANIMCHANNEL_ALL, "fire", 0);
		return SRESULT_STAGE(STAGE_WAIT);

	case STAGE_WAIT:
		if ((!gameLocal.isMultiplayer && (wsfl.lowerWeapon || AnimDone(ANIMCHANNEL_ALL, 0))) || AnimDone(ANIMCHANNEL_ALL, 0))
		{
			SetState("Idle", 0);
			return SRESULT_DONE;
		}
		if (wsfl.attack && gameLocal.time >= nextAttackTime && AmmoInClip())
		{
			SetState("Fire", 0);
			return SRESULT_DONE;
		}
		if (clipSize)
		{
			if ((wsfl.netReload || (wsfl.reload && AmmoInClip() < ClipSize() && AmmoAvailable() > AmmoInClip())))
			{
				SetState("Reload", 4);
				return SRESULT_DONE;
			}
		}
		if (UpdateFlashlight())
		{
			return SRESULT_DONE;
		}
		return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponShotgun::State_Reload
================
*/
stateResult_t rvWeaponShotgun::State_Reload(const stateParms_t &parms)
{
	enum
	{
		STAGE_INIT,
		STAGE_WAIT,
		STAGE_RELOADSTARTWAIT,
		STAGE_RELOADLOOP,
		STAGE_RELOADLOOPWAIT,
		STAGE_RELOADDONE,
		STAGE_RELOADDONEWAIT
	};
	switch (parms.stage)
	{
	case STAGE_INIT:
		if (wsfl.netReload)
		{
			wsfl.netReload = false;
		}
		else
		{
			NetReload();
		}

		SetStatus(WP_RELOAD);

		if (mods & SHOTGUN_MOD_AMMO)
		{
			PlayAnim(ANIMCHANNEL_ALL, "reload_clip", parms.blendFrames);
		}
		else
		{
			PlayAnim(ANIMCHANNEL_ALL, "reload_start", parms.blendFrames);
			return SRESULT_STAGE(STAGE_RELOADSTARTWAIT);
		}
		return SRESULT_STAGE(STAGE_WAIT);

	case STAGE_WAIT:
		if (AnimDone(ANIMCHANNEL_ALL, 4))
		{
			AddToClip(ClipSize());
			SetState("Idle", 4);
			return SRESULT_DONE;
		}
		if (wsfl.lowerWeapon)
		{
			SetState("Lower", 4);
			return SRESULT_DONE;
		}
		return SRESULT_WAIT;

	case STAGE_RELOADSTARTWAIT:
		if (AnimDone(ANIMCHANNEL_ALL, 0))
		{
			return SRESULT_STAGE(STAGE_RELOADLOOP);
		}
		if (wsfl.lowerWeapon)
		{
			SetState("Lower", 4);
			return SRESULT_DONE;
		}
		return SRESULT_WAIT;

	case STAGE_RELOADLOOP:
		if ((wsfl.attack && AmmoInClip()) || AmmoAvailable() <= AmmoInClip() || AmmoInClip() == ClipSize())
		{
			return SRESULT_STAGE(STAGE_RELOADDONE);
		}
		PlayAnim(ANIMCHANNEL_ALL, "reload_loop", 0);
		return SRESULT_STAGE(STAGE_RELOADLOOPWAIT);

	case STAGE_RELOADLOOPWAIT:
		if ((wsfl.attack && AmmoInClip()) || wsfl.netEndReload)
		{
			return SRESULT_STAGE(STAGE_RELOADDONE);
		}
		if (wsfl.lowerWeapon)
		{
			SetState("Lower", 4);
			return SRESULT_DONE;
		}
		if (AnimDone(ANIMCHANNEL_ALL, 0))
		{
			AddToClip(1);
			return SRESULT_STAGE(STAGE_RELOADLOOP);
		}
		return SRESULT_WAIT;

	case STAGE_RELOADDONE:
		NetEndReload();
		PlayAnim(ANIMCHANNEL_ALL, "reload_end", 0);
		return SRESULT_STAGE(STAGE_RELOADDONEWAIT);

	case STAGE_RELOADDONEWAIT:
		if (wsfl.lowerWeapon)
		{
			SetState("Lower", 4);
			return SRESULT_DONE;
		}
		if (wsfl.attack && AmmoInClip() && gameLocal.time > nextAttackTime)
		{
			SetState("Fire", 0);
			return SRESULT_DONE;
		}
		if (AnimDone(ANIMCHANNEL_ALL, 4))
		{
			SetState("Idle", 4);
			return SRESULT_DONE;
		}
		return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}


/*
================
rvWeaponShotgun::UpdateFlashlight
================
*/
bool rvWeaponShotgun::UpdateFlashlight(void)
{
	if (!wsfl.flashlight)
	{		
		return false;
	}

	SetState("Flashlight", 0);
	return true;
}

/*
================
rvWeaponShotgun	::Flashlight
================
*/
void rvWeaponShotgun::Flashlight(bool on)
{
	owner->Flashlight(on);

	if (on)
	{
		viewModel->ShowSurface("models/weapons/blaster/flare");
		worldModel->ShowSurface("models/weapons/blaster/flare");
	}
	else
	{
		viewModel->HideSurface("models/weapons/blaster/flare");
		worldModel->HideSurface("models/weapons/blaster/flare");
	}
}
stateResult_t rvWeaponShotgun::State_Flashlight(const stateParms_t &parms)
{
	enum
	{
		FLASHLIGHT_INIT,
		FLASHLIGHT_WAIT,
		FLASHLIGHT_DONE
	};
	switch (parms.stage)
	{
	case FLASHLIGHT_INIT:
		SetStatus(WP_FLASHLIGHT);
		// Wait for the flashlight anim to play
		PlayAnim(ANIMCHANNEL_ALL, "flashlight", 0);
		return SRESULT_STAGE(FLASHLIGHT_WAIT);

	case FLASHLIGHT_WAIT:
		if (!AnimDone(ANIMCHANNEL_ALL, 4))
		{
			return SRESULT_WAIT;
		}

		Flashlight(owner->IsFlashlightOn());

		return SRESULT_STAGE(FLASHLIGHT_DONE);

	case FLASHLIGHT_DONE:
		if (wsfl.flashlight)
		{
			return SRESULT_WAIT;
		}
		SetState("Idle", 4);
		return SRESULT_DONE;
	}
	return SRESULT_ERROR;
}