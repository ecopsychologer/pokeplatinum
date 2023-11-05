#include <nitro.h>
#include <string.h>

#include "constants/abilities.h"
#include "constants/battle.h"
#include "constants/flavor.h"
#include "constants/gender.h"
#include "constants/heap.h"
#include "constants/items.h"
#include "constants/species.h"
#include "constants/string.h"
#include "constants/narc_files/battle_skill_subseq.h"

#include "struct_decls/struct_party_decl.h"
#include "struct_decls/battle_system.h"
#include "struct_decls/struct_02098700_decl.h"
#include "struct_defs/fraction.h"
#include "overlay016/struct_ov16_0225BFFC_decl.h"

#include "battle/battle_context.h"
#include "battle/battle_controller.h"
#include "battle/battle_message.h"
#include "battle/battle_mon.h"
#include "battle/btlcmd.h"
#include "battle/common.h"

#include "unk_020021B0.h"
#include "narc.h"
#include "heap.h"
#include "strbuf.h"
#include "trainer_info.h"
#include "unk_020366A0.h"
#include "pokemon.h"
#include "move_table.h"
#include "trainer_data.h"
#include "party.h"
#include "item.h"
#include "unk_0208C098.h"
#include "unk_02098700.h"
#include "unk_02098988.h"
#include "flags.h"
#include "overlay016/ov16_0223DF00.h"
#include "overlay016/ov16_0225177C.h"
#include "overlay016/ov16_0225CBB8.h"
#include "overlay016/ov16_0226485C.h"

static BOOL ov16_02254EF4(BattleContext * param0, int param1, int param2, int param3);
static int MapSideEffectToSubscript(BattleContext *battleCtx, enum SideEffectType type, u32 effect);
static int ov16_0225B63C(BattleContext * param0, int param1, int param2, int param3, int param4, u32 * param5);
static BOOL ov16_022553F8(BattleContext * param0, int param1, int param2);
static void ov16_02255448(int param0, u32 * param1);
static BOOL ov16_0225B6C8(BattleContext * param0, int param1);
static u8 Battler_MonType(BattleContext *battleCtx, int battler, enum BattleMonParam paramID);
static void BattleAI_ClearKnownMoves(BattleContext *battleCtx, u8 battler);
static void BattleAI_ClearKnownAbility(BattleContext *battleCtx, u8 battler);
static void BattleAI_ClearKnownItem(BattleContext *battleCtx, u8 battler);
static int ChooseTraceTarget(BattleSystem *battleSys, BattleContext *battleCtx, int defender1, int defender2);
static BOOL MoveCannotTriggerAnticipation(BattleContext * param0, int param1);
static int BattleMove_Type(BattleSystem * param0, BattleContext * param1, int param2, int param3);

static const Fraction sStatStageBoosts[];

void BattleSystem_InitBattleMon(BattleSystem *battleSys, BattleContext *battleCtx, int battler, int partySlot)
{
    Pokemon *mon = BattleSystem_PartyPokemon(battleSys, battler, partySlot);

    battleCtx->battleMons[battler].species = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);
    battleCtx->battleMons[battler].attack = Pokemon_GetValue(mon, MON_DATA_ATK, NULL);
    battleCtx->battleMons[battler].defense = Pokemon_GetValue(mon, MON_DATA_DEF, NULL);
    battleCtx->battleMons[battler].speed = Pokemon_GetValue(mon, MON_DATA_SPEED, NULL);
    battleCtx->battleMons[battler].spAttack = Pokemon_GetValue(mon, MON_DATA_SP_ATK, NULL);
    battleCtx->battleMons[battler].spDefense = Pokemon_GetValue(mon, MON_DATA_SP_DEF, NULL);

    int i;
    for (i = 0; i < LEARNED_MOVES_MAX; i++) {
        battleCtx->battleMons[battler].moves[i] = Pokemon_GetValue(mon, MON_DATA_MOVE1 + i, NULL);
        battleCtx->battleMons[battler].ppCur[i] = Pokemon_GetValue(mon, MON_DATA_MOVE1_CUR_PP + i, NULL);
        battleCtx->battleMons[battler].ppUps[i] = Pokemon_GetValue(mon, MON_DATA_MOVE1_PP_UPS + i, NULL);
    }

    battleCtx->battleMons[battler].hpIV = Pokemon_GetValue(mon, MON_DATA_HP_IV, NULL);
    battleCtx->battleMons[battler].attackIV = Pokemon_GetValue(mon, MON_DATA_ATK_IV, NULL);
    battleCtx->battleMons[battler].defenseIV = Pokemon_GetValue(mon, MON_DATA_DEF_IV, NULL);
    battleCtx->battleMons[battler].speedIV = Pokemon_GetValue(mon, MON_DATA_SPEED_IV, NULL);
    battleCtx->battleMons[battler].spAttackIV = Pokemon_GetValue(mon, MON_DATA_SPATK_IV, NULL);
    battleCtx->battleMons[battler].spDefenseIV = Pokemon_GetValue(mon, MON_DATA_SPDEF_IV, NULL);
    battleCtx->battleMons[battler].isEgg = Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL);
    battleCtx->battleMons[battler].hasNickname = Pokemon_GetValue(mon, MON_DATA_HAS_NICKNAME, NULL);

    if ((battleCtx->battleStatusMask & SYSCTL_BATON_PASS) == FALSE) {
        for (i = 0; i < BATTLE_STAT_MAX; i++) {
            battleCtx->battleMons[battler].statBoosts[i] = 6;
        }
    }

    battleCtx->battleMons[battler].weatherAbilityAnnounced = FALSE;
    battleCtx->battleMons[battler].intimidateAnnounced = FALSE;
    battleCtx->battleMons[battler].traceAnnounced = FALSE;
    battleCtx->battleMons[battler].downloadAnnounced = FALSE;
    battleCtx->battleMons[battler].anticipationAnnounced = FALSE;
    battleCtx->battleMons[battler].forewarnAnnounced = FALSE;
    battleCtx->battleMons[battler].slowStartAnnounced = FALSE;
    battleCtx->battleMons[battler].slowStartFinished = FALSE;
    battleCtx->battleMons[battler].friskAnnounced = FALSE;
    battleCtx->battleMons[battler].moldBreakerAnnounced = FALSE;
    battleCtx->battleMons[battler].pressureAnnounced = FALSE;
    battleCtx->battleMons[battler].type1 = Pokemon_GetValue(mon, MON_DATA_TYPE_1, NULL);
    battleCtx->battleMons[battler].type2 = Pokemon_GetValue(mon, MON_DATA_TYPE_2, NULL);
    battleCtx->battleMons[battler].gender = Pokemon_GetGender(mon);
    battleCtx->battleMons[battler].isShiny = Pokemon_IsShiny(mon);

    if (BattleSystem_BattleType(battleSys) & BATTLE_TYPE_NO_ABILITIES) {
        battleCtx->battleMons[battler].ability = ABILITY_NONE;
        battleCtx->battleMons[battler].status = MON_CONDITION_NONE;
        battleCtx->battleMons[battler].heldItem = ITEM_NONE;
    } else {
        battleCtx->battleMons[battler].ability = Pokemon_GetValue(mon, MON_DATA_ABILITY, NULL);
        battleCtx->battleMons[battler].status = Pokemon_GetValue(mon, MON_DATA_STATUS_CONDITION, NULL);
        battleCtx->battleMons[battler].heldItem = Pokemon_GetValue(mon, MON_DATA_HELD_ITEM, NULL);
    }

    if ((BattleSystem_BattleType(battleSys) & BATTLE_TYPE_NO_ABILITIES) && Battler_Side(battleSys, battler) == BATTLER_US) {
        battleCtx->battleMons[battler].formNum = 0;
    } else {
        battleCtx->battleMons[battler].formNum = Pokemon_GetValue(mon, MON_DATA_FORM, NULL);
    }

    battleCtx->battleMons[battler].level = Pokemon_GetValue(mon, MON_DATA_LEVEL, NULL);
    battleCtx->battleMons[battler].friendship = Pokemon_GetValue(mon, MON_DATA_FRIENDSHIP, NULL);
    battleCtx->battleMons[battler].curHP = Pokemon_GetValue(mon, MON_DATA_CURRENT_HP, NULL);
    battleCtx->battleMons[battler].maxHP = Pokemon_GetValue(mon, MON_DATA_MAX_HP, NULL);
    battleCtx->battleMons[battler].exp = Pokemon_GetValue(mon, MON_DATA_EXP, NULL);
    battleCtx->battleMons[battler].personality = Pokemon_GetValue(mon, MON_DATA_PERSONALITY, NULL);
    battleCtx->battleMons[battler].OTId = Pokemon_GetValue(mon, MON_DATA_OT_ID, NULL);
    battleCtx->battleMons[battler].OTGender = Pokemon_GetValue(mon, MON_DATA_OT_GENDER, NULL);
    battleCtx->battleMons[battler].capturedBall = Pokemon_GetValue(mon, MON_DATA_POKEBALL, NULL);

    Pokedex_SetupGiratina(battleCtx->battleMons[battler].formNum);

    HeightWeightData *heightWeightData = Pokedex_HeightWeightData(HEAP_ID_BATTLE);
    Pokedex_HeightWeightData_Load(heightWeightData, 0, HEAP_ID_BATTLE);

    battleCtx->battleMons[battler].weight = Pokedex_HeightWeightData_Weight(heightWeightData, battleCtx->battleMons[battler].species);

    Pokedex_HeightWeightData_Release(heightWeightData);
    Pokedex_HeightWeightData_Free(heightWeightData);
    Pokemon_GetValue(mon, MON_DATA_NICKNAME, battleCtx->battleMons[battler].nickname);
    Pokemon_GetValue(mon, MON_DATA_OTNAME, battleCtx->battleMons[battler].OTName);

    battleCtx->battleMons[battler].timesDamaged = 0;
    battleCtx->battleMons[battler].trainerMessageFlags = 0;

    int side = Battler_Side(battleSys, battler);
    if (battleCtx->sideConditions[side].knockedOffItemsMask & FlagIndex(battleCtx->selectedPartySlot[battler])) {
        battleCtx->battleMons[battler].heldItem = ITEM_NONE;
        battleCtx->battleMons[battler].moveEffectsData.canUnburden = FALSE;
    } else if (battleCtx->battleMons[battler].heldItem) {
        battleCtx->battleMons[battler].moveEffectsData.canUnburden = TRUE;
    }
}

void BattleSystem_ReloadPokemon(BattleSystem *battleSys, BattleContext *battleCtx, int battler, int partySlot)
{
    Pokemon *mon = BattleSystem_PartyPokemon(battleSys, battler, partySlot);

    battleCtx->battleMons[battler].attack = Pokemon_GetValue(mon, MON_DATA_ATK, NULL);
    battleCtx->battleMons[battler].defense = Pokemon_GetValue(mon, MON_DATA_DEF, NULL);
    battleCtx->battleMons[battler].speed = Pokemon_GetValue(mon, MON_DATA_SPEED, NULL);
    battleCtx->battleMons[battler].spAttack = Pokemon_GetValue(mon, MON_DATA_SP_ATK, NULL);
    battleCtx->battleMons[battler].spDefense = Pokemon_GetValue(mon, MON_DATA_SP_DEF, NULL);
    battleCtx->battleMons[battler].level = Pokemon_GetValue(mon, MON_DATA_LEVEL, NULL);
    battleCtx->battleMons[battler].friendship = Pokemon_GetValue(mon, MON_DATA_FRIENDSHIP, NULL);
    battleCtx->battleMons[battler].curHP = Pokemon_GetValue(mon, MON_DATA_CURRENT_HP, NULL);
    battleCtx->battleMons[battler].maxHP = Pokemon_GetValue(mon, MON_DATA_MAX_HP, NULL);

    if ((battleCtx->battleMons[battler].statusVolatile & VOLATILE_CONDITION_TRANSFORM) == FALSE) {
        for (int i = 0; i < LEARNED_MOVES_MAX; i++) {
            if ((battleCtx->battleMons[battler].moveEffectsData.mimickedMoveSlot & FlagIndex(i)) == FALSE) {
                battleCtx->battleMons[battler].moves[i] = Pokemon_GetValue(mon, MON_DATA_MOVE1 + i, NULL);
                battleCtx->battleMons[battler].ppCur[i] = Pokemon_GetValue(mon, MON_DATA_MOVE1_CUR_PP + i, NULL);
                battleCtx->battleMons[battler].ppUps[i] = Pokemon_GetValue(mon, MON_DATA_MOVE1_PP_UPS + i, NULL);
            }
        }

        battleCtx->battleMons[battler].exp = Pokemon_GetValue(mon, MON_DATA_EXP, NULL);
    }
}

void BattleSystem_LoadScript(BattleContext *battleCtx, int narc, int file)
{
    GF_ASSERT(NARC_GetMemberSizeByIndexPair(narc, file) < BATTLE_SCRIPT_SIZE_MAX * sizeof(u32));

    battleCtx->scriptNarc = narc;
    battleCtx->scriptFile = file;
    battleCtx->scriptCursor = 0;

    NARC_ReadWholeMemberByIndexPair(&battleCtx->battleScript, narc, file);
}

void BattleSystem_CallScript(BattleContext *battleCtx, int narc, int file)
{
    GF_ASSERT(NARC_GetMemberSizeByIndexPair(narc, file) < 400 * 4);
    GF_ASSERT(battleCtx->scriptStackPointer < 4);

    battleCtx->scriptStackNarc[battleCtx->scriptStackPointer] = battleCtx->scriptNarc;
    battleCtx->scriptStackFile[battleCtx->scriptStackPointer] = battleCtx->scriptFile;
    battleCtx->scriptStackCursor[battleCtx->scriptStackPointer] = battleCtx->scriptCursor;
    battleCtx->scriptStackPointer++;
    battleCtx->scriptNarc = narc;
    battleCtx->scriptFile = file;
    battleCtx->scriptCursor = 0;

    NARC_ReadWholeMemberByIndexPair(&battleCtx->battleScript, narc, file);
}

BOOL BattleSystem_PopScript(BattleContext *battleCtx)
{
    if (battleCtx->scriptStackPointer) {
        battleCtx->scriptStackPointer--;
        BattleSystem_LoadScript(battleCtx, battleCtx->scriptStackNarc[battleCtx->scriptStackPointer], battleCtx->scriptStackFile[battleCtx->scriptStackPointer]);

        battleCtx->scriptCursor = battleCtx->scriptStackCursor[battleCtx->scriptStackPointer];
        return FALSE;
    }
    
    return TRUE;
}

void ov16_02251F44 (BattleContext * param0, int param1, int param2, int param3)
{
    int v0;

    for (v0 = 0; v0 < 16; v0++) {
        if (param0->ioQueue[param1][param2][v0] == 0) {
            param0->ioQueue[param1][param2][v0] = param3;
            break;
        }
    }

    GF_ASSERT(v0 < 16);
}

void ov16_02251F80 (BattleContext * param0, int param1, int param2, int param3)
{
    int v0;

    GF_ASSERT(param3 != 0);

    for (v0 = 0; v0 < 16; v0++) {
        if (param0->ioQueue[param1][param2][v0] == param3) {
            param0->ioQueue[param1][param2][v0] = 0;
            break;
        }
    }

    GF_ASSERT(v0 < 16);
}

BOOL BattleIO_QueueIsEmpty(BattleContext *battleCtx)
{
    int linkedPlayer, battler, j;
    int sumData = 0;

    for (linkedPlayer = 0; linkedPlayer < MAX_LINK_BATTLERS; linkedPlayer++) {
        for (battler = 0; battler < MAX_BATTLERS; battler++) {
            for (j = 0; j < BATTLE_IO_QUEUE_SIZE; j++) {
                sumData += battleCtx->ioQueue[linkedPlayer][battler][j];
            }
        }
    }

    if (sumData == 0) {
        battleCtx->linkBattleTimeout = 0;
    }

    return sumData == 0;
}

void BattleIO_UpdateTimeout(BattleContext *battleCtx)
{
    battleCtx->linkBattleTimeout++;
    if (battleCtx->linkBattleTimeout > LINK_BATTLE_TIMEOUT) {
        Link_SetErrorState(LINK_BATTLE_RESET_SAVEPOINT);
    }
}

void BattleIO_ClearBuffer(BattleContext *battleCtx, int battler)
{
    for (int i = 0; i < BATTLE_IO_BUFFER_SIZE; i++) {
        battleCtx->ioBuffer[battler][i] = 0;
    }
}

int BattleMon_Get(BattleContext *battleCtx, int battler, enum BattleMonParam paramID, void *buf)
{
    BattleMon *battleMon = &battleCtx->battleMons[battler];

    switch (paramID) {
    case BATTLEMON_SPECIES:
        return battleMon->species;

    case BATTLEMON_ATTACK:
        return battleMon->attack;

    case BATTLEMON_DEFENSE:
        return battleMon->defense;

    case BATTLEMON_SPEED:
        return battleMon->speed;

    case BATTLEMON_SP_ATTACK:
        return battleMon->spAttack;

    case BATTLEMON_SP_DEFENSE:
        return battleMon->spDefense;

    case BATTLEMON_MOVE_1:
    case BATTLEMON_MOVE_2:
    case BATTLEMON_MOVE_3:
    case BATTLEMON_MOVE_4:
        return battleMon->moves[paramID - BATTLEMON_MOVE_1];

    case BATTLEMON_HP_IV:
        return battleMon->hpIV;

    case BATTLEMON_ATTACK_IV:
        return battleMon->attackIV;

    case BATTLEMON_DEFENSE_IV:
        return battleMon->defenseIV;

    case BATTLEMON_SPEED_IV:
        return battleMon->speedIV;

    case BATTLEMON_SP_ATTACK_IV:
        return battleMon->spAttackIV;

    case BATTLEMON_SP_DEFENSE_IV:
        return battleMon->spDefenseIV;

    case BATTLEMON_IS_EGG:
        return battleMon->isEgg;

    case BATTLEMON_HAS_NICKNAME:
        return battleMon->hasNickname;

    case BATTLEMON_HP_STAGE:
    case BATTLEMON_ATTACK_STAGE:
    case BATTLEMON_DEFENSE_STAGE:
    case BATTLEMON_SPEED_STAGE:
    case BATTLEMON_SP_ATTACK_STAGE:
    case BATTLEMON_SP_DEFENSE_STAGE:
    case BATTLEMON_ACCURACY_STAGE:
    case BATTLEMON_EVASION_STAGE:
        return battleMon->statBoosts[paramID - BATTLEMON_HP_STAGE];

    case BATTLEMON_ABILITY:
        return battleMon->ability;

    case BATTLEMON_TYPE_1:
    case BATTLEMON_TYPE_2:
        return Battler_MonType(battleCtx, battler, paramID);

    case BATTLEMON_GENDER:
        return battleMon->gender;

    case BATTLEMON_IS_SHINY:
        return battleMon->isShiny;

    case BATTLEMON_CUR_PP_1:
    case BATTLEMON_CUR_PP_2:
    case BATTLEMON_CUR_PP_3:
    case BATTLEMON_CUR_PP_4:
        return battleMon->ppCur[paramID - BATTLEMON_CUR_PP_1];

    case BATTLEMON_PP_UPS_1:
    case BATTLEMON_PP_UPS_2:
    case BATTLEMON_PP_UPS_3:
    case BATTLEMON_PP_UPS_4:
        return battleMon->ppUps[paramID - BATTLEMON_PP_UPS_1];

    case BATTLEMON_MAX_PP_1:
    case BATTLEMON_MAX_PP_2:
    case BATTLEMON_MAX_PP_3:
    case BATTLEMON_MAX_PP_4:
        return MoveTable_CalcMaxPP(battleMon->moves[paramID - BATTLEMON_MAX_PP_1], battleMon->ppUps[paramID - BATTLEMON_MAX_PP_1]);

    case BATTLEMON_LEVEL:
        return battleMon->level;

    case BATTLEMON_FRIENDSHIP:
        return battleMon->friendship;

    case BATTLEMON_NICKNAME:
        {
            int i;
            charcode_t *nickname = buf;

            for (i = 0; i < MON_NAME_LEN + 1; i++) {
                nickname[i] = battleMon->nickname[i];
            }
        }
        break;

    case BATTLEMON_NICKNAME_STRBUF:
        Strbuf_CopyChars((Strbuf *)buf, battleMon->nickname);
        break;

    case BATTLEMON_CUR_HP:
        return battleMon->curHP;

    case BATTLEMON_MAX_HP:
        return battleMon->maxHP;

    case BATTLEMON_OT_NAME:
        {
            int i;
            charcode_t *OTName = buf;

            for (i = 0; i < MON_NAME_LEN + 1; i++) {
                OTName[i] = battleMon->OTName[i];
            }
        }
        break;

    case BATTLEMON_EXP:
        return battleMon->exp;

    case BATTLEMON_PERSONALITY:
        return battleMon->personality;

    case BATTLEMON_STATUS:
        return battleMon->status;

    case BATTLEMON_VOLATILE_STATUS:
        return battleMon->statusVolatile;

    case BATTLEMON_OT_ID:
        return battleMon->OTId;

    case BATTLEMON_HELD_ITEM:
        return battleMon->heldItem;

    case BATTLEMON_TIMES_DAMAGED:
        return battleMon->timesDamaged;

    case BATTLEMON_TRAINER_MESSAGE_FLAGS:
        return battleMon->trainerMessageFlags;

    case BATTLEMON_OT_GENDER:
        return battleMon->OTGender;

    case BATTLEMON_MOVE_EFFECTS_MASK:
        return battleMon->moveEffectsMask;

    case BATTLEMON_MOVE_EFFECTS_TEMP:
        return battleMon->moveEffectsTemp;

    case BATTLEMON_DISABLED_TURNS:
        return battleMon->moveEffectsData.disabledTurns;

    case BATTLEMON_ENCORED_TURNS:
        return battleMon->moveEffectsData.encoredTurns;

    case BATTLEMON_CHARGED_TURNS:
        return battleMon->moveEffectsData.chargedTurns;

    case BATTLEMON_TAUNTED_TURNS:
        return battleMon->moveEffectsData.tauntedTurns;

    case BATTLEMON_SUCCESSFUL_PROTECT_TURNS:
        return battleMon->moveEffectsData.protectSuccessTurns;

    case BATTLEMON_PERISH_SONG_TURNS:
        return battleMon->moveEffectsData.perishSongTurns;

    case BATTLEMON_ROLLOUT_COUNT:
        return battleMon->moveEffectsData.rolloutCount;

    case BATTLEMON_FURY_CUTTER_COUNT:
        return battleMon->moveEffectsData.furyCutterCount;

    case BATTLEMON_STOCKPILE_COUNT:
        return battleMon->moveEffectsData.stockpileCount;

    case BATTLEMON_STOCKPILE_DEF_BOOSTS:
        return battleMon->moveEffectsData.stockpileDefBoosts;

    case BATTLEMON_STOCKPILE_SPDEF_BOOSTS:
        return battleMon->moveEffectsData.stockpileSpDefBoosts;

    case BATTLEMON_TRUANT:
        return battleMon->moveEffectsData.truant;

    case BATTLEMON_FLASH_FIRE:
        return battleMon->moveEffectsData.flashFire;

    case BATTLEMON_LOCK_ON_TARGET:
        return battleMon->moveEffectsData.lockOnTarget;

    case BATTLEMON_MIMICKED_MOVE_SLOT:
        return battleMon->moveEffectsData.mimickedMoveSlot;

    case BATTLEMON_BIND_TARGET:
        return battleMon->moveEffectsData.bindTarget;

    case BATTLEMON_MEAN_LOOK_TARGET:
        return battleMon->moveEffectsData.meanLookTarget;

    case BATTLEMON_LAST_RESORT_COUNT:
        return battleMon->moveEffectsData.lastResortCount;

    case BATTLEMON_MAGNET_RISE_TURNS:
        return battleMon->moveEffectsData.magnetRiseTurns;

    case BATTLEMON_HEAL_BLOCK_TURNS:
        return battleMon->moveEffectsData.healBlockTurns;

    case BATTLEMON_EMBARGO_TURNS:
        return battleMon->moveEffectsData.embargoTurns;

    case BATTLEMON_CAN_UNBURDEN:
        return battleMon->moveEffectsData.canUnburden;

    case BATTLEMON_METRONOME_TURNS:
        return battleMon->moveEffectsData.metronomeTurns;

    case BATTLEMON_MICLE_BERRY:
        return battleMon->moveEffectsData.micleBerry;

    case BATTLEMON_CUSTAP_BERRY:
        return battleMon->moveEffectsData.custapBerry;

    case BATTLEMON_QUICK_CLAW:
        return battleMon->moveEffectsData.quickClaw;

    case BATTLEMON_RECHARGE_TURN_NUMBER:
        return battleMon->moveEffectsData.rechargeTurnNumber;

    case BATTLEMON_FAKE_OUT_TURN_NUMBER:
        return battleMon->moveEffectsData.fakeOutTurnNumber;

    case BATTLEMON_SLOW_START_TURN_NUMBER:
        return battleMon->moveEffectsData.slowStartTurnNumber;

    case BATTLEMON_SUBSTITUTE_HP:
        return battleMon->moveEffectsData.substituteHP;

    case BATTLEMON_TRANSFORMED_PERSONALITY:
        return battleMon->moveEffectsData.transformedPID;

    case BATTLEMON_DISABLED_MOVE:
        return battleMon->moveEffectsData.disabledMove;

    case BATTLEMON_ENCORED_MOVE:
        return battleMon->moveEffectsData.encoredMove;

    case BATTLEMON_BINDING_MOVE:
        return battleMon->moveEffectsData.bindingMove;

    case BATTLEMON_ITEM_HP_RECOVERY:
        return battleMon->moveEffectsData.itemHPRecovery;

    case BATTLEMON_SLOW_START_ANNOUNCED:
        return battleMon->slowStartAnnounced;

    case BATTLEMON_SLOW_START_FINISHED:
        return battleMon->slowStartFinished;

    case BATTLEMON_FORM_NUM:
        return battleMon->formNum;

    case BATTLEMON_TEMP:
        return BattleMon_Get(battleCtx, battler, battleCtx->scriptTemp, buf);

    default:
        GF_ASSERT(FALSE);
        break;
    }

    return 0;
}

void BattleMon_Set(BattleContext *battleCtx, int battler, enum BattleMonParam paramID, const void *buf)
{
    BattleMon *mon = &battleCtx->battleMons[battler];

    switch (paramID) {
    case BATTLEMON_SPECIES:
        mon->species = *(u16 *)buf;
        break;

    case BATTLEMON_ATTACK:
        mon->attack = *(u16 *)buf;
        break;

    case BATTLEMON_DEFENSE:
        mon->defense = *(u16 *)buf;
        break;

    case BATTLEMON_SPEED:
        mon->speed = *(u16 *)buf;
        break;

    case BATTLEMON_SP_ATTACK:
        mon->spAttack = *(u16 *)buf;
        break;

    case BATTLEMON_SP_DEFENSE:
        mon->spDefense = *(u16 *)buf;
        break;

    case BATTLEMON_MOVE_1:
    case BATTLEMON_MOVE_2:
    case BATTLEMON_MOVE_3:
    case BATTLEMON_MOVE_4:
        mon->moves[paramID - BATTLEMON_MOVE_1] = *(u16 *)buf;
        break;

    case BATTLEMON_HP_IV:
        mon->hpIV = *(u8 *)buf;
        break;

    case BATTLEMON_ATTACK_IV:
        mon->attackIV = *(u8 *)buf;
        break;

    case BATTLEMON_DEFENSE_IV:
        mon->defenseIV = *(u8 *)buf;
        break;

    case BATTLEMON_SPEED_IV:
        mon->speedIV = *(u8 *)buf;
        break;

    case BATTLEMON_SP_ATTACK_IV:
        mon->spAttackIV = *(u8 *)buf;
        break;

    case BATTLEMON_SP_DEFENSE_IV:
        mon->spDefenseIV = *(u8 *)buf;
        break;

    case BATTLEMON_IS_EGG:
        mon->isEgg = *(u8 *)buf;
        break;

    case BATTLEMON_HAS_NICKNAME:
        mon->hasNickname = *(u8 *)buf;
        break;

    case BATTLEMON_HP_STAGE:
    case BATTLEMON_ATTACK_STAGE:
    case BATTLEMON_DEFENSE_STAGE:
    case BATTLEMON_SPEED_STAGE:
    case BATTLEMON_SP_ATTACK_STAGE:
    case BATTLEMON_SP_DEFENSE_STAGE:
    case BATTLEMON_ACCURACY_STAGE:
    case BATTLEMON_EVASION_STAGE:
        mon->statBoosts[paramID - BATTLEMON_HP_STAGE] = *(s8 *)buf;
        break;

    case BATTLEMON_ABILITY:
        mon->ability = *(u8 *)buf;
        break;

    case BATTLEMON_TYPE_1:
        mon->type1 = *(u8 *)buf;
        break;

    case BATTLEMON_TYPE_2:
        mon->type2 = *(u8 *)buf;
        break;

    case BATTLEMON_GENDER:
        mon->gender = *(u8 *)buf;
        break;

    case BATTLEMON_IS_SHINY:
        mon->isShiny = *(u8 *)buf;
        break;

    case BATTLEMON_CUR_PP_1:
    case BATTLEMON_CUR_PP_2:
    case BATTLEMON_CUR_PP_3:
    case BATTLEMON_CUR_PP_4:
        mon->ppCur[paramID - BATTLEMON_CUR_PP_1] = *(u8 *)buf;
        break;

    case BATTLEMON_PP_UPS_1:
    case BATTLEMON_PP_UPS_2:
    case BATTLEMON_PP_UPS_3:
    case BATTLEMON_PP_UPS_4:
        mon->ppUps[paramID - BATTLEMON_PP_UPS_1] = *(u8 *)buf;
        break;
        
    case BATTLEMON_MAX_PP_1:
    case BATTLEMON_MAX_PP_2:
    case BATTLEMON_MAX_PP_3:
    case BATTLEMON_MAX_PP_4:
        GF_ASSERT(FALSE);
        break;

    case BATTLEMON_LEVEL:
        mon->level = *(u8 *)buf;
        break;

    case BATTLEMON_FRIENDSHIP:
        mon->friendship = *(u8 *)buf;
        break;
        
    case BATTLEMON_NICKNAME:
        for (int i = 0; i < MON_NAME_LEN + 1; i++) {
            mon->nickname[i] = ((u16 *)buf)[i];
        }
        break;

    case BATTLEMON_CUR_HP:
        mon->curHP = *(s16 *)buf;
        break;

    case BATTLEMON_MAX_HP:
        mon->maxHP = *(u16 *)buf;
        break;

    case BATTLEMON_OT_NAME:
        for (int i = 0; i < MON_NAME_LEN + 1; i++) {
            mon->OTName[i] = ((u16 *)buf)[i];
        }
        break;

    case BATTLEMON_EXP:
        mon->exp = *(u32 *)buf;
        break;

    case BATTLEMON_PERSONALITY:
        mon->personality = *(u32 *)buf;
        break;
    
    case BATTLEMON_STATUS:
        mon->status = *(u32 *)buf;
        break;

    case BATTLEMON_VOLATILE_STATUS:
        mon->statusVolatile = *(u32 *)buf;
        break;

    case BATTLEMON_OT_ID:
        mon->OTId = *(u32 *)buf;
        break;

    case BATTLEMON_HELD_ITEM:
        mon->heldItem = *(u16 *)buf;
        break;

    case BATTLEMON_TIMES_DAMAGED:
        mon->timesDamaged = *(u8 *)buf;
        break;

    case BATTLEMON_TRAINER_MESSAGE_FLAGS:
        mon->trainerMessageFlags = *(u8 *)buf;
        break;

    case BATTLEMON_OT_GENDER:
        mon->OTGender = *(u8 *)buf;
        break;
    case BATTLEMON_MOVE_EFFECTS_MASK:
        mon->moveEffectsMask = *(u32 *)buf;
        break;

    case BATTLEMON_MOVE_EFFECTS_TEMP:
        mon->moveEffectsTemp = *(u32 *)buf;
        break;

    case BATTLEMON_DISABLED_TURNS:
        mon->moveEffectsData.disabledTurns = *(u8 *)buf;
        break;

    case BATTLEMON_ENCORED_TURNS:
        mon->moveEffectsData.encoredTurns = *(u8 *)buf;
        break;

    case BATTLEMON_CHARGED_TURNS:
        mon->moveEffectsData.chargedTurns = *(u8 *)buf;
        break;

    case BATTLEMON_TAUNTED_TURNS:
        mon->moveEffectsData.tauntedTurns = *(u8 *)buf;
        break;

    case BATTLEMON_SUCCESSFUL_PROTECT_TURNS:
        mon->moveEffectsData.protectSuccessTurns = *(u8 *)buf;
        break;

    case BATTLEMON_PERISH_SONG_TURNS:
        mon->moveEffectsData.perishSongTurns = *(u8 *)buf;
        break;

    case BATTLEMON_ROLLOUT_COUNT:
        mon->moveEffectsData.rolloutCount = *(u8 *)buf;
        break;

    case BATTLEMON_FURY_CUTTER_COUNT:
        mon->moveEffectsData.furyCutterCount = *(u8 *)buf;
        break;

    case BATTLEMON_STOCKPILE_COUNT:
        mon->moveEffectsData.stockpileCount = *(u8 *)buf;
        break;

    case BATTLEMON_STOCKPILE_DEF_BOOSTS:
        mon->moveEffectsData.stockpileDefBoosts = *(u8 *)buf;
        break;

    case BATTLEMON_STOCKPILE_SPDEF_BOOSTS:
        mon->moveEffectsData.stockpileSpDefBoosts = *(u8 *)buf;
        break;

    case BATTLEMON_TRUANT:
        mon->moveEffectsData.truant = *(u8 *)buf;
        break;

    case BATTLEMON_FLASH_FIRE:
        mon->moveEffectsData.flashFire = *(u8 *)buf;
        break;

    case BATTLEMON_LOCK_ON_TARGET:
        mon->moveEffectsData.lockOnTarget = *(u8 *)buf;
        break;

    case BATTLEMON_MIMICKED_MOVE_SLOT:
        mon->moveEffectsData.mimickedMoveSlot = *(u8 *)buf;
        break;

    case BATTLEMON_BIND_TARGET:
        mon->moveEffectsData.bindTarget = *(u8 *)buf;
        break;

    case BATTLEMON_MEAN_LOOK_TARGET:
        mon->moveEffectsData.meanLookTarget = *(u8 *)buf;
        break;

    case BATTLEMON_LAST_RESORT_COUNT:
        mon->moveEffectsData.lastResortCount = *(u8 *)buf;
        break;

    case BATTLEMON_MAGNET_RISE_TURNS:
        mon->moveEffectsData.magnetRiseTurns = *(u8 *)buf;
        break;

    case BATTLEMON_HEAL_BLOCK_TURNS:
        mon->moveEffectsData.healBlockTurns = *(u8 *)buf;
        break;

    case BATTLEMON_EMBARGO_TURNS:
        mon->moveEffectsData.embargoTurns = *(u8 *)buf;
        break;

    case BATTLEMON_CAN_UNBURDEN:
        mon->moveEffectsData.canUnburden = *(u8 *)buf;
        break;

    case BATTLEMON_METRONOME_TURNS:
        mon->moveEffectsData.metronomeTurns = *(u8 *)buf;
        break;

    case BATTLEMON_MICLE_BERRY:
        mon->moveEffectsData.micleBerry = *(u8 *)buf;
        break;

    case BATTLEMON_CUSTAP_BERRY:
        mon->moveEffectsData.custapBerry = *(u8 *)buf;
        break;

    case BATTLEMON_QUICK_CLAW:
        mon->moveEffectsData.quickClaw = *(u8 *)buf;
        break;

    case BATTLEMON_RECHARGE_TURN_NUMBER:
        mon->moveEffectsData.rechargeTurnNumber = *(u32 *)buf;
        break;

    case BATTLEMON_FAKE_OUT_TURN_NUMBER:
        mon->moveEffectsData.fakeOutTurnNumber = *(u32 *)buf;
        break;

    case BATTLEMON_SLOW_START_TURN_NUMBER:
        mon->moveEffectsData.slowStartTurnNumber = *(u32 *)buf;
        break;

    case BATTLEMON_SUBSTITUTE_HP:
        mon->moveEffectsData.substituteHP = *(u32 *)buf;
        break;

    case BATTLEMON_TRANSFORMED_PERSONALITY:
        mon->moveEffectsData.transformedPID = *(u32 *)buf;
        break;

    case BATTLEMON_DISABLED_MOVE:
        mon->moveEffectsData.disabledMove = *(u16 *)buf;
        break;

    case BATTLEMON_ENCORED_MOVE:
        mon->moveEffectsData.encoredMove = *(u16 *)buf;
        break;

    case BATTLEMON_BINDING_MOVE:
        mon->moveEffectsData.bindingMove = *(u16 *)buf;
        break;

    case BATTLEMON_ITEM_HP_RECOVERY:
        mon->moveEffectsData.itemHPRecovery = *(u32 *)buf;
        break;

    case BATTLEMON_SLOW_START_ANNOUNCED:
        mon->slowStartAnnounced = *(u8 *)buf;
        break;

    case BATTLEMON_SLOW_START_FINISHED:
        mon->slowStartFinished = *(u8 *)buf;
        break;

    case BATTLEMON_FORM_NUM:
        mon->formNum = *(u8 *)buf;
        break;

    case BATTLEMON_TEMP:
        BattleMon_Set(battleCtx, battler, battleCtx->scriptTemp, buf);
        break;

    default:
        GF_ASSERT(FALSE);
        break;
    }
}

void Battler_AddVal(BattleContext *battleCtx, int battler, enum BattleMonParam paramID, int val)
{
    BattleMon_AddVal(&battleCtx->battleMons[battler], paramID, val);
}

void BattleMon_AddVal(BattleMon *mon, enum BattleMonParam paramID, int val)
{
    switch (paramID) {
    case BATTLEMON_ATTACK:
        mon->attack += val;
        break;

    case BATTLEMON_DEFENSE:
        mon->defense += val;
        break;

    case BATTLEMON_SPEED:
        mon->speed += val;
        break;

    case BATTLEMON_SP_ATTACK:
        mon->spAttack += val;
        break;

    case BATTLEMON_SP_DEFENSE:
        mon->spDefense += val;
        break;

    case BATTLEMON_HP_IV:
        mon->hpIV += val;
        break;

    case BATTLEMON_ATTACK_IV:
        mon->attackIV += val;
        break;

    case BATTLEMON_DEFENSE_IV:
        mon->defenseIV += val;
        break;

    case BATTLEMON_SPEED_IV:
        mon->speedIV += val;
        break;

    case BATTLEMON_SP_ATTACK_IV:
        mon->spAttackIV += val;
        break;

    case BATTLEMON_SP_DEFENSE_IV:
        mon->spDefenseIV += val;
        break;

    case BATTLEMON_HP_STAGE:
    case BATTLEMON_ATTACK_STAGE:
    case BATTLEMON_DEFENSE_STAGE:
    case BATTLEMON_SPEED_STAGE:
    case BATTLEMON_SP_ATTACK_STAGE:
    case BATTLEMON_SP_DEFENSE_STAGE:
    case BATTLEMON_ACCURACY_STAGE:
    case BATTLEMON_EVASION_STAGE:
        if (mon->statBoosts[paramID - BATTLEMON_HP_STAGE] + val < 0) {
            mon->statBoosts[paramID - BATTLEMON_HP_STAGE] = 0;
        } else if (mon->statBoosts[paramID - BATTLEMON_HP_STAGE] + val > 12) {
            mon->statBoosts[paramID - BATTLEMON_HP_STAGE] = 12;
        } else {
            mon->statBoosts[paramID - BATTLEMON_HP_STAGE] += val;
        }
        break;

    case BATTLEMON_CUR_PP_1:
    case BATTLEMON_CUR_PP_2:
    case BATTLEMON_CUR_PP_3:
    case BATTLEMON_CUR_PP_4:
        int maxPP = MoveTable_CalcMaxPP(mon->moves[paramID - BATTLEMON_CUR_PP_1], mon->ppUps[paramID - BATTLEMON_CUR_PP_1]);
        if (mon->ppCur[paramID - BATTLEMON_CUR_PP_1] + val > maxPP) {
            mon->ppCur[paramID - BATTLEMON_CUR_PP_1] = maxPP;
        } else {
            mon->ppCur[paramID - BATTLEMON_CUR_PP_1] += val;
        }
        break;

    case BATTLEMON_PP_UPS_1:
    case BATTLEMON_PP_UPS_2:
    case BATTLEMON_PP_UPS_3:
    case BATTLEMON_PP_UPS_4:
        mon->ppUps[paramID - BATTLEMON_PP_UPS_1] += val;
        break;

    case BATTLEMON_LEVEL:
        mon->level += val;
        break;

    case BATTLEMON_FRIENDSHIP:
        int friendship = mon->friendship;

        if (friendship + val > 255) {
            friendship = 255;
        } else if (friendship + val < 0) {
            friendship = 0;
        } else {
            friendship += val;
        }

        mon->friendship = friendship;
        break;

    case BATTLEMON_CUR_HP:
        if (mon->curHP + val > mon->maxHP) {
            mon->curHP = mon->maxHP;
        } else {
            mon->curHP += val;
        }
        break;

    case BATTLEMON_MAX_HP:
        mon->maxHP += val;
        break;

    case BATTLEMON_EXP:
        mon->exp += val;
        break;

    case BATTLEMON_PERSONALITY:
        mon->personality += val;
        break;

    case BATTLEMON_DISABLED_TURNS:
        mon->moveEffectsData.disabledTurns += val;
        break;

    case BATTLEMON_ENCORED_TURNS:
        mon->moveEffectsData.encoredTurns += val;
        break;

    case BATTLEMON_CHARGED_TURNS:
        mon->moveEffectsData.chargedTurns += val;
        break;

    case BATTLEMON_TAUNTED_TURNS:
        mon->moveEffectsData.tauntedTurns += val;
        break;

    case BATTLEMON_SUCCESSFUL_PROTECT_TURNS:
        mon->moveEffectsData.protectSuccessTurns += val;
        break;

    case BATTLEMON_PERISH_SONG_TURNS:
        mon->moveEffectsData.perishSongTurns += val;
        break;

    case BATTLEMON_ROLLOUT_COUNT:
        mon->moveEffectsData.rolloutCount += val;
        break;

    case BATTLEMON_FURY_CUTTER_COUNT:
        mon->moveEffectsData.furyCutterCount += val;
        break;

    case BATTLEMON_STOCKPILE_COUNT:
        mon->moveEffectsData.stockpileCount += val;
        break;

    case BATTLEMON_STOCKPILE_DEF_BOOSTS:
        mon->moveEffectsData.stockpileDefBoosts += val;
        break;

    case BATTLEMON_STOCKPILE_SPDEF_BOOSTS:
        mon->moveEffectsData.stockpileSpDefBoosts += val;
        break;

    case BATTLEMON_LAST_RESORT_COUNT:
        mon->moveEffectsData.lastResortCount += val;
        break;

    case BATTLEMON_MAGNET_RISE_TURNS:
        mon->moveEffectsData.magnetRiseTurns += val;
        break;

    case BATTLEMON_HEAL_BLOCK_TURNS:
        mon->moveEffectsData.healBlockTurns += val;
        break;

    case BATTLEMON_RECHARGE_TURN_NUMBER:
        mon->moveEffectsData.rechargeTurnNumber += val;
        break;

    case BATTLEMON_FAKE_OUT_TURN_NUMBER:
        mon->moveEffectsData.fakeOutTurnNumber += val;
        break;

    case BATTLEMON_SLOW_START_TURN_NUMBER:
        mon->moveEffectsData.slowStartTurnNumber += val;
        break;

    case BATTLEMON_SUBSTITUTE_HP:
        mon->moveEffectsData.substituteHP += val;
        break;

    case BATTLEMON_ITEM_HP_RECOVERY:
        mon->moveEffectsData.itemHPRecovery += val;
        break;

    case BATTLEMON_SLOW_START_ANNOUNCED:
        mon->slowStartAnnounced += val;
        break;

    case BATTLEMON_SLOW_START_FINISHED:
        mon->slowStartFinished += val;
        break;

    case BATTLEMON_FORM_NUM:
        mon->formNum += val;
        break;

    default:
        GF_ASSERT(FALSE);
        break;
    }
}

static const u8 sSpeedHalvingItemEffects[] = {
    HOLD_EFFECT_EVS_UP_SPEED_DOWN,
    HOLD_EFFECT_SPEED_DOWN_GROUNDED,
    HOLD_EFFECT_LVLUP_HP_EV_UP,
    HOLD_EFFECT_LVLUP_ATK_EV_UP,
    HOLD_EFFECT_LVLUP_DEF_EV_UP,
    HOLD_EFFECT_LVLUP_SPEED_EV_UP,
    HOLD_EFFECT_LVLUP_SPATK_EV_UP,
    HOLD_EFFECT_LVLUP_SPDEF_EV_UP
};

static inline int CompareSpeed_ApplySimple(BattleContext *battleCtx, int battler, int stage)
{
    if (Battler_Ability(battleCtx, battler) == ABILITY_SIMPLE) {
        stage = 6 + ((stage - 6) * 2);

        if (stage > 12) {
            stage = 12;
        }

        if (stage < 0) {
            stage = 0;
        }
    }

    return stage;
}

enum {
    BATTLER_1_GOES_FIRST = 0,
    BATTLER_2_GOES_FIRST,
    BATTLER_2_WINS_SPEED_TIE,
};

u8 BattleSystem_CompareBattlerSpeed(BattleSystem *battleSys, BattleContext *battleCtx, int battler1, int battler2, BOOL ignoreQuickClaw)
{
    u8 result = BATTLER_1_GOES_FIRST;
    u32 battler1Speed, battler2Speed;
    u16 battler1Move = 0, battler2Move = 0;
    u8 battler1ItemEffect, battler1ItemParam;
    u8 battler2ItemEffect, battler2ItemParam;
    s8 battler1Priority = 0, battler2Priority = 0;
    u8 battler1QuickClaw = 0, battler2QuickClaw = 0;
    u8 battler1LaggingTail = 0, battler2LaggingTail = 0;
    int battler1Action, battler2Action;
    int battler1MoveSlot, battler2MoveSlot;
    int battler1Ability, battler2Ability;
    int battler1SpeedStage, battler2SpeedStage;
    int i;

    // If either battler is dead, short-circuit to preferring the other
    if (battleCtx->battleMons[battler1].curHP == 0 && battleCtx->battleMons[battler2].curHP) {
        return 1;
    }

    if (battleCtx->battleMons[battler1].curHP && battleCtx->battleMons[battler2].curHP == 0) {
        return 0;
    }

    battler1Ability = Battler_Ability(battleCtx, battler1);
    battler2Ability = Battler_Ability(battleCtx, battler2);
    battler1ItemEffect = Battler_HeldItemEffect(battleCtx, battler1);
    battler1ItemParam = Battler_HeldItemPower(battleCtx, battler1, ITEM_POWER_CHECK_ALL);
    battler2ItemEffect = Battler_HeldItemEffect(battleCtx, battler2);
    battler2ItemParam = Battler_HeldItemPower(battleCtx, battler2, ITEM_POWER_CHECK_ALL);
    battler1SpeedStage = battleCtx->battleMons[battler1].statBoosts[BATTLE_STAT_SPEED];
    battler2SpeedStage = battleCtx->battleMons[battler2].statBoosts[BATTLE_STAT_SPEED];

    battler1SpeedStage = CompareSpeed_ApplySimple(battleCtx, battler1, battler1SpeedStage);
    battler2SpeedStage = CompareSpeed_ApplySimple(battleCtx, battler2, battler2SpeedStage);

    battler1Speed = battleCtx->battleMons[battler1].speed * sStatStageBoosts[battler1SpeedStage].numerator / sStatStageBoosts[battler1SpeedStage].denominator;
    battler2Speed = battleCtx->battleMons[battler2].speed * sStatStageBoosts[battler2SpeedStage].numerator / sStatStageBoosts[battler2SpeedStage].denominator;

    if (NO_CLOUD_NINE) {
        if ((battler1Ability == ABILITY_SWIFT_SWIM && WEATHER_IS_RAIN)
                || (battler1Ability == ABILITY_CHLOROPHYLL && WEATHER_IS_SUN)) {
            battler1Speed *= 2;
        }

        if ((battler2Ability == ABILITY_SWIFT_SWIM && WEATHER_IS_RAIN)
                || (battler2Ability == ABILITY_CHLOROPHYLL && WEATHER_IS_SUN)) {
            battler2Speed *= 2;
        }
    }

    for (i = 0; i < NELEMS(sSpeedHalvingItemEffects); i++) {
        // The speed-halving effect of these items are not ignored by any negation effect
        if (BattleSystem_GetItemData(battleCtx, battleCtx->battleMons[battler1].heldItem, ITEM_PARAM_HOLD_EFFECT) == sSpeedHalvingItemEffects[i]) {
            battler1Speed /= 2;
            break;
        }
    }

    if (battler1ItemEffect == HOLD_EFFECT_CHOICE_SPEED) {
        battler1Speed = battler1Speed * 15 / 10;
    }

    if (battler1ItemEffect == HOLD_EFFECT_DITTO_SPEED_UP
            && battleCtx->battleMons[battler1].species == SPECIES_DITTO) {
        battler1Speed *= 2;
    }

    if (battler1Ability == ABILITY_QUICK_FEET && (battleCtx->battleMons[battler1].status & MON_CONDITION_ANY)) {
        battler1Speed = battler1Speed * 15 / 10;
    } else if (battleCtx->battleMons[battler1].status & MON_CONDITION_PARALYSIS) {
        battler1Speed /= 4;
    }

    if (battler1Ability == ABILITY_SLOW_START
            && battleCtx->totalTurns - battleCtx->battleMons[battler1].moveEffectsData.slowStartTurnNumber < 5) {
        battler1Speed /= 2;
    }

    if (battler1Ability == ABILITY_UNBURDEN
            && battleCtx->battleMons[battler1].moveEffectsData.canUnburden
            && battleCtx->battleMons[battler1].heldItem == ITEM_NONE) {
        battler1Speed *= 2;
    }

    if (battleCtx->sideConditionsMask[Battler_Side(battleSys, battler1)] & SIDE_CONDITION_TAILWIND) {
        battler1Speed *= 2;
    }

    if (battler1ItemEffect == HOLD_EFFECT_SOMETIMES_PRIORITY) {
        if (battleCtx->speedRand[battler1] % (100 / battler1ItemParam) == 0) {
            battler1QuickClaw = 1;

            if (ignoreQuickClaw == FALSE) {
                battleCtx->battleMons[battler1].moveEffectsData.quickClaw = 1;
            }
        }
    }

    if (battler1ItemEffect == HOLD_EFFECT_PINCH_PRIORITY) {
        if (Battler_Ability(battleCtx, battler1) == ABILITY_GLUTTONY) {
            battler1ItemParam /= 2;
        }

        if (battleCtx->battleMons[battler1].curHP <= (battleCtx->battleMons[battler1].maxHP / battler1ItemParam)) {
            battler1QuickClaw = 1;

            if (ignoreQuickClaw == FALSE) {
                battleCtx->battleMons[battler1].moveEffectsData.custapBerry = 1;
            }
        }
    }

    if (battler1ItemEffect == HOLD_EFFECT_PRIORITY_DOWN) {
        battler1LaggingTail = 1;
    }

    for (i = 0; i < NELEMS(sSpeedHalvingItemEffects); i++) {
        // The speed-halving effect of these items are not ignored by any negation effect
        if (BattleSystem_GetItemData(battleCtx, battleCtx->battleMons[battler2].heldItem, ITEM_PARAM_HOLD_EFFECT) == sSpeedHalvingItemEffects[i]) {
            battler2Speed /= 2;
            break;
        }
    }

    if (battler2ItemEffect == HOLD_EFFECT_CHOICE_SPEED) {
        battler2Speed = battler2Speed * 15 / 10;
    }

    if (battler2ItemEffect == HOLD_EFFECT_DITTO_SPEED_UP
            && battleCtx->battleMons[battler2].species == SPECIES_DITTO) {
        battler2Speed *= 2;
    }

    if (battler2Ability == ABILITY_QUICK_FEET && (battleCtx->battleMons[battler2].status & MON_CONDITION_ANY)) {
        battler2Speed = battler2Speed * 15 / 10;
    } else if (battleCtx->battleMons[battler2].status & MON_CONDITION_PARALYSIS) {
        battler2Speed /= 4;
    }

    if (battler2Ability == ABILITY_SLOW_START
            && battleCtx->totalTurns - battleCtx->battleMons[battler2].moveEffectsData.slowStartTurnNumber < 5) {
        battler2Speed /= 2;
    }

    if (battler2Ability == ABILITY_UNBURDEN
            && battleCtx->battleMons[battler2].moveEffectsData.canUnburden
            && battleCtx->battleMons[battler2].heldItem == ITEM_NONE) {
        battler2Speed *= 2;
    }

    if (battleCtx->sideConditionsMask[Battler_Side(battleSys, battler2)] & SIDE_CONDITION_TAILWIND) {
        battler2Speed *= 2;
    }

    if (battler2ItemEffect == HOLD_EFFECT_SOMETIMES_PRIORITY) {
        if (battleCtx->speedRand[battler2] % (100 / battler2ItemParam) == 0) {
            battler2QuickClaw = 1;

            if (ignoreQuickClaw == FALSE) {
                battleCtx->battleMons[battler2].moveEffectsData.quickClaw = 1;
            }
        }
    }

    if (battler2ItemEffect == HOLD_EFFECT_PINCH_PRIORITY) {
        if (Battler_Ability(battleCtx, battler2) == ABILITY_GLUTTONY) {
            battler2ItemParam /= 2;
        }

        if (battleCtx->battleMons[battler2].curHP <= (battleCtx->battleMons[battler2].maxHP / battler2ItemParam)) {
            battler2QuickClaw = 1;

            if (ignoreQuickClaw == FALSE) {
                battleCtx->battleMons[battler2].moveEffectsData.custapBerry = 1;
            }
        }
    }

    if (battler2ItemEffect == HOLD_EFFECT_PRIORITY_DOWN) {
        battler2LaggingTail = 1;
    }

    battleCtx->monSpeedValues[battler1] = battler1Speed;
    battleCtx->monSpeedValues[battler2] = battler2Speed;

    if (ignoreQuickClaw == FALSE) {
        battler1Action = battleCtx->battlerActions[battler1][BATTLE_ACTION_SELECTED_COMMAND];
        battler2Action = battleCtx->battlerActions[battler2][BATTLE_ACTION_SELECTED_COMMAND];
        battler1MoveSlot = battleCtx->moveSlot[battler1];
        battler2MoveSlot = battleCtx->moveSlot[battler2];

        if (battler1Action == PLAYER_INPUT_FIGHT) {
            if (battleCtx->turnFlags[battler1].struggling) {
                battler1Move = MOVE_STRUGGLE;
            } else {
                battler1Move = BattleMon_Get(battleCtx, battler1, BATTLEMON_MOVE_1 + battler1MoveSlot, NULL);
            }
        }

        if (battler2Action == PLAYER_INPUT_FIGHT) {
            if (battleCtx->turnFlags[battler2].struggling) {
                battler2Move = MOVE_STRUGGLE;
            } else {
                battler2Move = BattleMon_Get(battleCtx, battler2, BATTLEMON_MOVE_1 + battler2MoveSlot, NULL);
            }
        }

        battler1Priority = MOVE_DATA(battler1Move).priority;
        battler2Priority = MOVE_DATA(battler2Move).priority;
    }

    if (battler1Priority == battler2Priority) {
        if (battler1QuickClaw && battler2QuickClaw) {
            if (battler1Speed < battler2Speed) {
                result = BATTLER_2_GOES_FIRST;
            } else if (battler1Speed == battler2Speed && (BattleSystem_RandNext(battleSys) & 1)) {
                result = BATTLER_2_WINS_SPEED_TIE;
            }
        } else if (battler1QuickClaw == FALSE && battler2QuickClaw) {
            result = BATTLER_2_GOES_FIRST;
        } else if (battler1QuickClaw && battler2QuickClaw == FALSE) {
            result = BATTLER_1_GOES_FIRST;
        } else if (battler1LaggingTail && battler2LaggingTail) {
            if (battler1Speed > battler2Speed) {
                result = BATTLER_2_GOES_FIRST;
            } else if (battler1Speed == battler2Speed && (BattleSystem_RandNext(battleSys) & 1)) {
                result = BATTLER_2_WINS_SPEED_TIE;
            }
        } else if (battler1LaggingTail && battler2LaggingTail == FALSE) {
            result = BATTLER_2_GOES_FIRST;
        } else if (battler1LaggingTail == FALSE && battler2LaggingTail) {
            result = BATTLER_1_GOES_FIRST;
        } else if (battler1Ability == ABILITY_STALL && battler2Ability == ABILITY_STALL) {
            if (battler1Speed > battler2Speed) {
                result = BATTLER_2_GOES_FIRST;
            } else if (battler1Speed == battler2Speed && (BattleSystem_RandNext(battleSys) & 1)) {
                result = BATTLER_2_WINS_SPEED_TIE;
            }
        } else if (battler1Ability == ABILITY_STALL && battler2Ability != ABILITY_STALL) {
            result = BATTLER_2_GOES_FIRST;
        } else if (battler1Ability != ABILITY_STALL && battler2Ability == ABILITY_STALL) {
            result = BATTLER_1_GOES_FIRST;
        } else if (battleCtx->fieldConditionsMask & FIELD_CONDITION_TRICK_ROOM) {
            if (battler1Speed > battler2Speed) {
                result = BATTLER_2_GOES_FIRST;
            }

            if (battler1Speed == battler2Speed && (BattleSystem_RandNext(battleSys) & 1)) {
                result = BATTLER_2_WINS_SPEED_TIE;
            }
        } else {
            if (battler1Speed < battler2Speed) {
                result = BATTLER_2_GOES_FIRST;
            }

            if (battler1Speed == battler2Speed && (BattleSystem_RandNext(battleSys) & 1)) {
                result = BATTLER_2_WINS_SPEED_TIE;
            }
        }
    } else if (battler1Priority < battler2Priority) {
        result = BATTLER_2_GOES_FIRST;
    }

    return result;
}

void BattleSystem_ClearSideExpGain(BattleContext *battleCtx, int battler)
{
    battleCtx->sideGetExpMask[(battler >> 1) & 1] = 0;
}

void BattleSystem_FlagBattlerExpGain(BattleSystem *battleSys, BattleContext *battleCtx, int battler)
{
    int side = 0;
    u32 battleType = BattleSystem_BattleType(battleSys);

    while (side <= 2) {
        if ((battleCtx->battlersSwitchingMask & FlagIndex(side)) == FALSE
                && (battleCtx->battlersSwitchingMask & FlagIndex(battler)) == FALSE
                && battleCtx->battleMons[battler].curHP) {
            battleCtx->sideGetExpMask[(battler >> 1) & 1] |= FlagIndex(battleCtx->selectedPartySlot[side]);
        }

        side += 2;

        if (battleType == BATTLE_TYPE_AI_PARTNER || battleType == BATTLE_TYPE_TRAINER_WITH_AI_PARTNER) {
            break;
        }
    }
}

BOOL BattleSystem_TriggerPrimaryEffect(BattleSystem *battleSys, BattleContext *battleCtx, int *effect)
{
    BOOL result = FALSE;

    if (battleCtx->sideEffectDirectFlags & MOVE_SIDE_EFFECT_ON_HIT) {
        *effect = MapSideEffectToSubscript(battleCtx, 1, battleCtx->sideEffectDirectFlags);
        battleCtx->sideEffectDirectFlags = 0;

        if ((battleCtx->moveStatusFlags & MOVE_STATUS_NO_EFFECTS) == FALSE) {
            result = TRUE;
        }
    } else if (battleCtx->sideEffectDirectFlags) {
        *effect = MapSideEffectToSubscript(battleCtx, 1, battleCtx->sideEffectDirectFlags);

        if (battleCtx->battleMons[battleCtx->sideEffectMon].curHP
                && ((battleCtx->moveStatusFlags & MOVE_STATUS_NO_EFFECTS) == FALSE
                    // Brick Break's effect still applies even if the target is immune.
                    || ((battleCtx->sideEffectDirectFlags & MOVE_SIDE_EFFECT_BREAK_SCREENS)
                        && (battleCtx->moveStatusFlags & MOVE_STATUS_DID_NOT_AFFECT))
                    // Various other moves can still apply their effect even if the target
                    // is semi-invulnerable.
                    || ((battleCtx->sideEffectDirectFlags & MOVE_SIDE_EFFECT_CHECK_HP)
                        && (battleCtx->moveStatusFlags & MOVE_STATUS_GENERAL_MISS)))) {
            result = TRUE;
        }

        battleCtx->sideEffectDirectFlags = 0;
    }

    return result;
}

static inline void SetupSideEffect(BattleContext *battleCtx, int *effect, int source)
{
    *effect = MapSideEffectToSubscript(battleCtx, source, battleCtx->sideEffectIndirectFlags);
    battleCtx->sideEffectIndirectFlags = 0;
}

BOOL BattleSystem_TriggerSecondaryEffect(BattleSystem *battleSys, BattleContext *battleCtx, int *effect)
{
    BOOL result = FALSE;
    u16 effectChance;

    if (battleCtx->sideEffectIndirectFlags & MOVE_SIDE_EFFECT_ON_HIT) {
        SetupSideEffect(battleCtx, effect, SIDE_EFFECT_TYPE_INDIRECT);

        if ((battleCtx->moveStatusFlags & MOVE_STATUS_NO_EFFECTS) == FALSE) {
            result = TRUE;
        }
    } else if (battleCtx->sideEffectIndirectFlags & MOVE_SIDE_EFFECT_CHECK_SUBSTITUTE) {
        SetupSideEffect(battleCtx, effect, SIDE_EFFECT_TYPE_INDIRECT);

        if (Battler_SubstituteWasHit(battleCtx, battleCtx->sideEffectMon) == FALSE
                && (battleCtx->moveStatusFlags & MOVE_STATUS_NO_EFFECTS) == FALSE) {
            result = TRUE;
        }
    } else if (battleCtx->sideEffectIndirectFlags & MOVE_SIDE_EFFECT_CHECK_HP_AND_SUBSTITUTE) {
        SetupSideEffect(battleCtx, effect, SIDE_EFFECT_TYPE_INDIRECT);

        if (battleCtx->battleMons[battleCtx->sideEffectMon].curHP
                && Battler_SubstituteWasHit(battleCtx, battleCtx->sideEffectMon) == FALSE
                && (battleCtx->moveStatusFlags & MOVE_STATUS_NO_EFFECTS) == FALSE) {
            result = TRUE;
        }
    } else if (battleCtx->sideEffectIndirectFlags & MOVE_SIDE_EFFECT_CHECK_HP) {
        SetupSideEffect(battleCtx, effect, SIDE_EFFECT_TYPE_INDIRECT);

        if (battleCtx->battleMons[battleCtx->sideEffectMon].curHP) {
            result = TRUE;
        }
    } else if (battleCtx->sideEffectIndirectFlags & MOVE_SIDE_EFFECT_PROBABILISTIC) {
        if (Battler_Ability(battleCtx, battleCtx->attacker) == ABILITY_SERENE_GRACE) {
            effectChance = CURRENT_MOVE_DATA.effectChance * 2;
        } else {
            effectChance = CURRENT_MOVE_DATA.effectChance;
        }

        GF_ASSERT(effectChance != 0);

        if (BattleSystem_RandNext(battleSys) % 100 < effectChance) {
            battleCtx->battleStatusMask |= SYSCTL_APPLY_SECONDARY_EFFECT;
        }

        SetupSideEffect(battleCtx, effect, SIDE_EFFECT_TYPE_INDIRECT);
        if (battleCtx->battleMons[battleCtx->sideEffectMon].curHP == 0) {
            battleCtx->battleStatusMask &= ~SYSCTL_APPLY_SECONDARY_EFFECT;
        }

        result = TRUE;
    } else if (battleCtx->sideEffectIndirectFlags) {
        if (Battler_Ability(battleCtx, battleCtx->attacker) == ABILITY_SERENE_GRACE) {
            effectChance = CURRENT_MOVE_DATA.effectChance * 2;
        } else {
            effectChance = CURRENT_MOVE_DATA.effectChance;
        }

        GF_ASSERT(effectChance != 0);

        if (BattleSystem_RandNext(battleSys) % 100 < effectChance) {
            SetupSideEffect(battleCtx, effect, SIDE_EFFECT_TYPE_INDIRECT);

            if (battleCtx->battleMons[battleCtx->sideEffectMon].curHP
                    && Battler_SubstituteWasHit(battleCtx, battleCtx->sideEffectMon) == FALSE
                    && (battleCtx->moveStatusFlags & MOVE_STATUS_NO_EFFECTS) == FALSE) {
                result = TRUE;
            }
        }
    } else if (battleCtx->sideEffectAbilityFlags) {
        *effect = MapSideEffectToSubscript(battleCtx, SIDE_EFFECT_TYPE_ABILITY, battleCtx->sideEffectAbilityFlags);
        battleCtx->sideEffectAbilityFlags = 0;

        if (battleCtx->battleMons[battleCtx->sideEffectMon].curHP) {
            result = TRUE;
        }
    }

    return result;
}

int BattleSystem_Defender(BattleSystem *battleSys, BattleContext *battleCtx, int attacker, u16 move, BOOL randomize, int inRange)
{
    int defender = BATTLER_NONE;

    int range;
    if (move) {
        range = MOVE_DATA(move).range;
    } else {
        range = inRange;
    }

    if (range == RANGE_ADJACENT_OPPONENTS) { // e.g., Acid, Blizzard
        int maxBattlers = BattleSystem_MaxBattlers(battleSys);
        BattlerData *battlerData = BattleSystem_BattlerData(battleSys, attacker);
        u8 attackerType = Battler_Type(battlerData);

        // Assign the first possible living target based on speed order
        for (battleCtx->battlerCounter = 0; battleCtx->battlerCounter < maxBattlers; battleCtx->battlerCounter++) {
            int battler = battleCtx->monSpeedOrder[battleCtx->battlerCounter];

            // Check that this battler is an enemy of the attacker
            if (battleCtx->battleMons[battler].curHP != 0) {
                battlerData = BattleSystem_BattlerData(battleSys, battler);
                if (((attackerType & BATTLER_TYPE_SOLO_ENEMY) && (Battler_Type(battlerData) & BATTLER_TYPE_SOLO_ENEMY) == FALSE)
                        || ((attackerType & BATTLER_TYPE_SOLO_ENEMY) == FALSE) && (Battler_Type(battlerData) & BATTLER_TYPE_SOLO_ENEMY)) {
                    defender = battler;
                    break;
                }
            }
        }

        if (battleCtx->battlerCounter != maxBattlers) {
            battleCtx->battlerCounter++;
        }
    } else if (range == RANGE_ALL_ADJACENT) { // e.g., Earthquake, Surf
        int maxBattlers = BattleSystem_MaxBattlers(battleSys);

        // Assign the first possible living target based on speed order
        for (battleCtx->battlerCounter = 0; battleCtx->battlerCounter < maxBattlers; battleCtx->battlerCounter++) {
            int battler = battleCtx->monSpeedOrder[battleCtx->battlerCounter];

            // Only care that this battler is not the attacker
            if (battleCtx->battleMons[battler].curHP != 0 && battler != attacker) {
                defender = battler;
                break;
            }
        }

        if (battleCtx->battlerCounter != maxBattlers) {
            battleCtx->battlerCounter++;
        }
    } else if (range == RANGE_USER_OR_ALLY && randomize == TRUE) { // e.g., Acupressure
        if ((BattleSystem_BattleType(battleSys) & BATTLE_TYPE_DOUBLES)
                && BattleSystem_RandNext(battleSys) % 2 == 0) {
            defender = BattleSystem_Partner(battleSys, attacker);
            if (battleCtx->battleMons[defender].curHP == 0) {
                defender = attacker;
            }
        } else {
            defender = attacker;
        }
    } else if (range == RANGE_SINGLE_TARGET_ME_FIRST && randomize == TRUE) { // e.g., Me First
        defender = BattleSystem_RandomOpponent(battleSys, battleCtx, attacker);
    } else if (range == RANGE_OPPONENT_SIDE) { // e.g., Spikes, Stealth Rock
        defender = BattleSystem_RandomOpponent(battleSys, battleCtx, attacker);
    } else if (range == RANGE_USER // e.g., Swords Dance
            || range == RANGE_USER_SIDE // e.g., Light Screen, Reflect
            || range == RANGE_SINGLE_TARGET_SPECIAL // e.g., Counter, Mirror Coat
            || range == RANGE_FIELD) { // e.g., Sunny Day
        defender = attacker;
    } else if (range == RANGE_ALLY) { // e.g., Helping Hand
        if (BattleSystem_BattleType(battleSys) & BATTLE_TYPE_DOUBLES) {
            defender = BattleSystem_Partner(battleSys, attacker);
        } else {
            defender = attacker;
        }
    } else if (range == RANGE_USER_OR_ALLY) { // e.g., Acupressure
        if (BattleSystem_BattleType(battleSys) & BATTLE_TYPE_DOUBLES) {
            defender = battleCtx->battlerActions[attacker][BATTLE_ACTION_CHOOSE_TARGET];
            if (battleCtx->battleMons[defender].curHP == 0) {
                defender = attacker;
            }
        } else {
            defender = attacker;
        }
    } else if (range == RANGE_RANDOM_OPPONENT || randomize == TRUE) { // e.g., Outrage, Thrash, any other reason the move should be randomly targeted
        int opponents[2];
        int battleType = BattleSystem_BattleType(battleSys);
        int enemySide = Battler_Side(battleSys, attacker) ^ 1;

        opponents[0] = BattleSystem_EnemyInSlot(battleSys, attacker, ENEMY_IN_SLOT_RIGHT);
        opponents[1] = BattleSystem_EnemyInSlot(battleSys, attacker, ENEMY_IN_SLOT_LEFT);

        if (battleType & BATTLE_TYPE_DOUBLES) {
            if (battleCtx->sideConditions[enemySide].followMe
                    && battleCtx->battleMons[battleCtx->sideConditions[enemySide].followMeUser].curHP) {
                // If Follow Me is active and the user is still alive, re-point all targets toward them
                defender = battleCtx->sideConditions[enemySide].followMeUser;
            } else if (battleCtx->battleMons[opponents[0]].curHP
                    && battleCtx->battleMons[opponents[1]].curHP) {
                defender = opponents[BattleSystem_RandNext(battleSys) & 1];
            } else if (battleCtx->battleMons[opponents[0]].curHP) {
                defender = opponents[0];
            } else if (battleCtx->battleMons[opponents[1]].curHP) {
                defender = opponents[1];
            }
        } else if (battleCtx->battleMons[attacker ^ 1].curHP) {
            defender = attacker ^ 1;
        }
    } else { // the usual single-target moves, e.g., Flamethrower, Thunderbolt
        int enemySide = Battler_Side(battleSys, attacker) ^ 1;
        int target = battleCtx->battlerActions[attacker][BATTLE_ACTION_CHOOSE_TARGET];
        int maxBattlers = BattleSystem_MaxBattlers(battleSys);

        if (battleCtx->sideConditions[enemySide].followMe
                && battleCtx->battleMons[battleCtx->sideConditions[enemySide].followMeUser].curHP) {
            // If Follow Me is active and the user is still alive, re-point all targets toward them
            defender = battleCtx->sideConditions[enemySide].followMeUser;
        } else if (battleCtx->battleMons[target].curHP) {
            defender = target;
        } else {
            // If the original target is no longer alive, try to target their partner instead
            target = BattleSystem_RandomOpponent(battleSys, battleCtx, attacker);
            if (battleCtx->battleMons[target].curHP) {
                defender = target;
            }
        }
    }

    return defender;
}

void BattleSystem_RedirectTarget (BattleSystem * param0, BattleContext * param1, int param2, u16 param3)
{
    int v0;
    int v1;
    int v2;
    int v3;
    int v4;

    if (param1->defender == 0xff) {
        return;
    }

    if ((Battler_Ability(param1, param2) == 96) || (Battler_Ability(param1, param2) == 104)) {
        return;
    }

    v0 = Battler_Side(param0, param2) ^ 1;

    if ((param1->sideConditions[v0].followMe) && (param1->battleMons[param1->sideConditions[v0].followMeUser].curHP)) {
        return;
    }

    v3 = BattleMove_Type(param0, param1, param2, param3);

    if (v3 == 0) {
        v3 = param1->aiContext.moveTable[param3].type;
    }

    v4 = BattleSystem_MaxBattlers(param0);

    if ((v3 == 13) && ((param1->aiContext.moveTable[param3].range == 0x0) || (param1->aiContext.moveTable[param3].range == 0x2)) && ((param1->battleStatusMask & 0x20) == 0) && (BattleSystem_CountAbility(param0, param1, 9, param2, 31))) {
        for (v1 = 0; v1 < v4; v1++) {
            v2 = param1->monSpeedOrder[v1];

            if ((Battler_Ability(param1, v2) == 31) && (param1->battleMons[v2].curHP) && (param2 != v2)) {
                break;
            }
        }

        if (v2 != param1->defender) {
            param1->selfTurnFlags[v2].lightningRodActivated = 1;
            param1->defender = v2;
        }
    } else if ((v3 == 11) && ((param1->aiContext.moveTable[param3].range == 0x0) || (param1->aiContext.moveTable[param3].range == 0x2)) && ((param1->battleStatusMask & 0x20) == 0) && (BattleSystem_CountAbility(param0, param1, 9, param2, 114))) {
        for (v1 = 0; v1 < v4; v1++) {
            v2 = param1->monSpeedOrder[v1];

            if ((Battler_Ability(param1, v2) == 114) && (param1->battleMons[v2].curHP) && (param2 != v2)) {
                break;
            }
        }

        if (v2 != param1->defender) {
            param1->selfTurnFlags[v2].stormDrainActivated = 1;
            param1->defender = v2;
        }
    }
}

BOOL BattleMove_TriggerRedirectionAbilities(BattleSystem *battleSys, BattleContext *battleCtx)
{
    BOOL result = FALSE;

    if ((battleCtx->moveStatusFlags & MOVE_STATUS_NO_EFFECTS) == FALSE && DEFENDER_SELF_TURN_FLAGS.lightningRodActivated) {
        battleCtx->selfTurnFlags[battleCtx->defender].lightningRodActivated = 0;

        LOAD_SUBSEQ(BATTLE_SUBSEQ_LIGHTNING_ROD_REDIRECTED);
        battleCtx->commandNext = battleCtx->command;
        battleCtx->command = BATTLE_CONTROL_EXEC_SCRIPT;

        result = TRUE;
    }

    if ((battleCtx->moveStatusFlags & MOVE_STATUS_NO_EFFECTS) == FALSE && DEFENDER_SELF_TURN_FLAGS.stormDrainActivated) {
        battleCtx->selfTurnFlags[battleCtx->defender].stormDrainActivated = 0;

        LOAD_SUBSEQ(BATTLE_SUBSEQ_LIGHTNING_ROD_REDIRECTED);
        battleCtx->commandNext = battleCtx->command;
        battleCtx->command = BATTLE_CONTROL_EXEC_SCRIPT;

        result = TRUE;
    }

    return result;
}

void BattleMon_CopyToParty (BattleSystem * param0, BattleContext * param1, int param2)
{
    if (param1->battleMons[param2].heldItem == 0) {
        BattleAI_ClearKnownItem(param1, param2);
    }

    BattleIO_UpdatePartyMon(param0, param1, param2);
}

void Battler_LockMoveChoice(BattleSystem *battleSys, BattleContext *battleCtx, int battler)
{
    battleCtx->battleMons[battler].statusVolatile |= VOLATILE_CONDITION_MOVE_LOCKED;
    battleCtx->moveLockedInto[battler] = battleCtx->moveCur;
}

void Battler_UnlockMoveChoice(BattleSystem *battleSys, BattleContext *battleCtx, int battler)
{
    battleCtx->battleMons[battler].statusVolatile &= ~VOLATILE_CONDITION_MOVE_LOCKED;
    battleCtx->battleMons[battler].statusVolatile &= ~VOLATILE_CONDITION_BIDE;
    battleCtx->battleMons[battler].moveEffectsMask &= ~MOVE_EFFECT_SEMI_INVULNERABLE;
    battleCtx->battleMons[battler].moveEffectsData.rolloutCount = 0;
    battleCtx->battleMons[battler].moveEffectsData.furyCutterCount = 0;
}

int ov16_02253F7C (BattleContext * param0, int param1)
{
    if (param0->battleMons[param1].status & 0x7) {
        return 1;
    } else if (param0->battleMons[param1].status & 0x8) {
        return 2;
    } else if (param0->battleMons[param1].status & 0x10) {
        return 3;
    } else if (param0->battleMons[param1].status & 0x20) {
        return 4;
    } else if (param0->battleMons[param1].status & 0x40) {
        return 5;
    } else if (param0->battleMons[param1].status & 0x80) {
        return 2;
    }

    return 0;
}

BOOL BattleSystem_CheckTrainerMessage (BattleSystem * param0, BattleContext * param1)
{
    int v0;
    int v1;
    int v2;

    v0 = BattleSystem_BattleType(param0);

    if (v0 & (0x4 | 0x80)) {
        return 0;
    }

    if ((v0 & 0x1) == 0) {
        return 0;
    }

    if (v0 & 0x2) {
        return 0;
    }

    v1 = ov16_0223E0D8(param0, 1);
    v2 = 0;

    do {
        switch (v2) {
        case 0:
            if ((param1->battleMons[1].timesDamaged == 1) && ((param1->battleStatusMask2 & 0x20) == 0)) {
                if (TrainerData_HasMessageType(v1, 13, 5)) {
                    param1->battleStatusMask2 |= 0x20;
                    param1->msgTemp = 13;
                    return 1;
                }
            }

            v2++;
            break;
        case 1:
            if ((param1->battleMons[1].trainerMessageFlags & 0x2) == 0) {
                if (param1->battleMons[1].curHP <= (param1->battleMons[1].maxHP / 2)) {
                    if (TrainerData_HasMessageType(v1, 14, 5)) {
                        param1->battleMons[1].trainerMessageFlags |= 0x2;
                        param1->msgTemp = 14;
                        return 1;
                    }
                }
            }

            v2++;
            break;
        case 2:
            if ((param1->battleMons[1].trainerMessageFlags & 0x3) == 0) {
                {
                    int v3;
                    int v4;
                    Party * v5;
                    Pokemon * v6;

                    v5 = BattleSystem_Party(param0, 1);
                    v4 = 0;

                    for (v3 = 0; v3 < Party_GetCurrentCount(v5); v3++) {
                        v6 = Party_GetPokemonBySlotIndex(v5, v3);

                        if (Pokemon_GetValue(v6, MON_DATA_CURRENT_HP, NULL)) {
                            v4++;
                        }
                    }

                    if (v4 == 1) {
                        if (TrainerData_HasMessageType(v1, 15, 5)) {
                            param1->battleMons[1].trainerMessageFlags |= 0x3;
                            param1->msgTemp = 15;
                            return 1;
                        }
                    }
                }
            }

            v2++;
            break;
        case 3:
            if ((param1->battleMons[1].trainerMessageFlags & 0x4) == 0) {
                {
                    int v7;
                    int v8;
                    Party * v9;
                    Pokemon * v10;

                    v9 = BattleSystem_Party(param0, 1);
                    v8 = 0;

                    for (v7 = 0; v7 < Party_GetCurrentCount(v9); v7++) {
                        v10 = Party_GetPokemonBySlotIndex(v9, v7);

                        if (Pokemon_GetValue(v10, MON_DATA_CURRENT_HP, NULL)) {
                            v8++;
                        }
                    }

                    if ((v8 == 1) && (param1->battleMons[1].curHP <= (param1->battleMons[1].maxHP / 2))) {
                        if (TrainerData_HasMessageType(v1, 16, 5)) {
                            param1->battleMons[1].trainerMessageFlags |= 0x4;
                            param1->msgTemp = 16;
                            return 1;
                        }
                    }
                }
            }

            v2++;
            break;
        case 4:
            break;
        }
    } while (v2 != 4);

    return 0;
}

void BattleContext_Init (BattleContext * param0)
{
    int v0;

    param0->damage = 0;
    param0->criticalMul = 1;
    param0->criticalBoosts = 0;
    param0->movePower = 0;
    param0->powerMul = 10;
    param0->moveType = 0;
    param0->moveEffectChance = 0;
    param0->moveStatusFlags = 0;
    param0->faintedMon = 0xff;
    param0->sideEffectDirectFlags = 0;
    param0->sideEffectIndirectFlags = 0;
    param0->sideEffectAbilityFlags = 0;
    param0->sideEffectType = 0;
    param0->sideEffectParam = 0x0;
    param0->sideEffectMon = 0xff;
    param0->multiHitCounter = 0;
    param0->multiHitNumHits = 0;
    param0->battlerCounter = 0;
    param0->multiHitLoop = 0;
    param0->afterMoveMessageType = 0;
    param0->multiHitCheckFlags = 0;
    param0->multiHitAccuracyCheck = 0;
    param0->fieldConditionCheckState = 0;
    param0->monConditionCheckState = 0;
    param0->sideConditionCheckState = 0;
    param0->turnStartCheckState = 0;
    param0->afterMoveHitCheckState = 0;
    param0->afterMoveMessageState = 0;
    param0->afterMoveEffectState = 0;
    param0->beforeMoveCheckState = 0;
    param0->tryMoveCheckState = 0;
    param0->statusCheckState = 0;
    param0->abilityCheckState = 0;
    param0->battleStatusMask &= ((1 | 2 | 4 | 8 | 16 | 32 | 256 | 512 | 1024 | 2048 | 0x80000 | 8192) | (4096 | 16384 | 32768 | 65536 | 0x20000 | 0x40000 | 0x100000 | 0x200000 | 0x400000 | 128 | 64)) ^ 0xffffffff;
    param0->battleStatusMask2 &= (2 | 4 | 8 | 16 | 64 | 256) ^ 0xffffffff;
    param0->magnitude = 0;

    for (v0 = 0; v0 < 4; v0++) {
        MI_CpuClearFast(&param0->selfTurnFlags[v0], sizeof(struct SelfTurnFlags));
        param0->aiSwitchedPartySlot[v0] = 6;
    }
}

void BattleContext_InitCounters (BattleSystem * param0, BattleContext * param1)
{
    int v0;
    int v1;

    for (v1 = 0; v1 < 4; v1++) {
        param1->moveHitBattler[v1] = 0xff;
        param1->switchedPartySlot[v1] = 6;
        param1->speedRand[v1] = BattleSystem_RandNext(param0);
    }

    param1->prizeMoneyMul = 1;
    param1->meFirstTurnOrder = 1;

    v0 = BattleSystem_BattleType(param0);

    if ((v0 & 0x2) == 0) {
        param1->battlersSwitchingMask |= FlagIndex(2);
        param1->battlersSwitchingMask |= FlagIndex(3);
    }

    param1->safariCatchStage = 6;
    param1->safariEscapeCount = 6;
}

void BattleSystem_UpdateAfterSwitch(BattleSystem *battleSys, BattleContext *battleCtx, int battler)
{
    // declare C89-style to match
    int i, maxBattlers;
    u32 battleType;
    u8 *addr;
    MoveEffectsData moveEffects;

    moveEffects = battleCtx->battleMons[battler].moveEffectsData;
    maxBattlers = BattleSystem_MaxBattlers(battleSys);
    battleType = BattleSystem_BattleType(battleSys);

    // Forcefully end the battler's turn after the replacement
    battleCtx->battlerActions[battler][BATTLE_ACTION_PICK_COMMAND] = BATTLE_CONTROL_MOVE_END;

    if ((battleCtx->battleStatusMask & SYSCTL_BATON_PASS) == FALSE) {
        // Clear any Mean Look or Lock On effects from other active battlers
        for (i = 0; i < maxBattlers; i++) {
            if ((battleCtx->battleMons[i].statusVolatile & VOLATILE_CONDITION_MEAN_LOOK)
                    && (battleCtx->battleMons[i].moveEffectsData.meanLookTarget == battler)) {
                battleCtx->battleMons[i].statusVolatile &= ~VOLATILE_CONDITION_MEAN_LOOK;
            }

            if ((battleCtx->battleMons[i].moveEffectsMask & MOVE_EFFECT_LOCK_ON)
                    && (battleCtx->battleMons[i].moveEffectsData.lockOnTarget == battler)) {
                battleCtx->battleMons[i].moveEffectsMask &= ~MOVE_EFFECT_LOCK_ON;
                battleCtx->battleMons[i].moveEffectsData.lockOnTarget = 0;
            }
        }

        battleCtx->battleMons[battler].statusVolatile = VOLATILE_CONDITION_NONE;
        battleCtx->battleMons[battler].moveEffectsMask = MOVE_EFFECT_NONE;
    } else {
        // Baton Pass maintains Focus Energy, Mean Look, Confusion, Curse, Substitute,
        // and a variety of move effects (see constants/battle/moves.h)
        battleCtx->battleMons[battler].statusVolatile &= VOLATILE_CONDITION_BATON_PASSED;
        battleCtx->battleMons[battler].moveEffectsMask &= MOVE_EFFECT_BATON_PASSED;

        for (i = 0; i < maxBattlers; i++) {
            if ((battleCtx->battleMons[i].moveEffectsMask & MOVE_EFFECT_LOCK_ON)
                    && battleCtx->battleMons[i].moveEffectsData.lockOnTarget == battler) {
                // When transferring Lock On due to Baton Pass, its effect timer is refreshed
                battleCtx->battleMons[i].moveEffectsMask &= ~MOVE_EFFECT_LOCK_ON;
                battleCtx->battleMons[i].moveEffectsMask |= MOVE_EFFECT_LOCK_ON_INITIAL_DURATION;
            }
        }
    }

    // Clear the effects of Attract and Bind sourced from the replaced battler
    for (i = 0; i < maxBattlers; i++) {
        if (battleCtx->battleMons[i].statusVolatile & (FlagIndex(battler) << VOLATILE_CONDITION_ATTRACT_SHIFT)) {
            battleCtx->battleMons[i].statusVolatile &= FLAG_NEGATE(FlagIndex(battler) << VOLATILE_CONDITION_ATTRACT_SHIFT);
        }

        if ((battleCtx->battleMons[i].statusVolatile & VOLATILE_CONDITION_BIND)
                && battleCtx->battleMons[i].moveEffectsData.bindTarget == battler) {
            battleCtx->battleMons[i].statusVolatile &= ~VOLATILE_CONDITION_BIND;
        }
    }

    // Clear all move effects for the battler
    addr = (u8 *)&battleCtx->battleMons[battler].moveEffectsData; // doesn't match with &moveEffectsData
    for (i = 0; i < sizeof(MoveEffectsData); i++) {
        addr[i] = 0;
    }

    // Update the move effects for Baton Pass, if applicable
    if (battleCtx->battleStatusMask & SYSCTL_BATON_PASS) {
        battleCtx->battleMons[battler].moveEffectsData.substituteHP = moveEffects.substituteHP;
        battleCtx->battleMons[battler].moveEffectsData.lockOnTarget = moveEffects.lockOnTarget;
        battleCtx->battleMons[battler].moveEffectsData.perishSongTurns = moveEffects.perishSongTurns;
        battleCtx->battleMons[battler].moveEffectsData.meanLookTarget = moveEffects.meanLookTarget;
        battleCtx->battleMons[battler].moveEffectsData.magnetRiseTurns = moveEffects.magnetRiseTurns;
        battleCtx->battleMons[battler].moveEffectsData.embargoTurns = moveEffects.embargoTurns;
        battleCtx->battleMons[battler].moveEffectsData.healBlockTurns = moveEffects.healBlockTurns;
    }

    battleCtx->battleMons[battler].moveEffectsData.fakeOutTurnNumber = battleCtx->totalTurns + 1;
    battleCtx->battleMons[battler].moveEffectsData.slowStartTurnNumber = battleCtx->totalTurns + 1;
    battleCtx->battleMons[battler].moveEffectsData.truant = (battleCtx->totalTurns + 1) & 1;
    battleCtx->moveProtect[battler] = MOVE_NONE;
    battleCtx->moveHit[battler] = MOVE_NONE;
    battleCtx->moveHitBattler[battler] = BATTLER_NONE;
    battleCtx->moveHitType[battler] = MOVE_NONE;
    battleCtx->movePrevByBattler[battler] = MOVE_NONE;
    battleCtx->moveCopied[battler] = MOVE_NONE;
    battleCtx->moveCopiedHit[battler][0] = MOVE_NONE;
    battleCtx->moveCopiedHit[battler][1] = MOVE_NONE;
    battleCtx->moveCopiedHit[battler][2] = MOVE_NONE;
    battleCtx->moveCopiedHit[battler][3] = MOVE_NONE;
    battleCtx->moveSketched[battler] = MOVE_NONE;
    battleCtx->conversion2Move[battler] = MOVE_NONE;
    battleCtx->conversion2Battler[battler] = MOVE_NONE;
    battleCtx->conversion2Type[battler] = MOVE_NONE;
    battleCtx->metronomeMove[battler] = MOVE_NONE;
    battleCtx->fieldConditionsMask &= FLAG_NEGATE(FlagIndex(battler) << FIELD_CONDITION_UPROAR_SHIFT);

    if (battleCtx->battleMons[battler].moveEffectsMask & MOVE_EFFECT_POWER_TRICK) {
        i = battleCtx->battleMons[battler].attack;
        battleCtx->battleMons[battler].attack = battleCtx->battleMons[battler].defense;
        battleCtx->battleMons[battler].defense = i;
    }

    for (i = 0; i < maxBattlers; i++) {
        if (i != battler && Battler_Side(battleSys, i) != Battler_Side(battleSys, battler)) {
            battleCtx->moveCopied[i] = MOVE_NONE;
        }

        battleCtx->moveCopiedHit[i][battler] = MOVE_NONE;
    }

    BattleAI_ClearKnownMoves(battleCtx, battler);
    BattleAI_ClearKnownAbility(battleCtx, battler);
    BattleAI_ClearKnownItem(battleCtx, battler);
}

void BattleSystem_CleanupFaintedMon(BattleSystem *battleSys, BattleContext *battleCtx, int battler)
{
    int maxBattlers = BattleSystem_MaxBattlers(battleSys);
    int i;

    for (i = BATTLE_STAT_HP; i < BATTLE_STAT_MAX; i++) {
        battleCtx->battleMons[battler].statBoosts[i] = 6;
    }

    battleCtx->battleMons[battler].statusVolatile = 0;
    battleCtx->battleMons[battler].moveEffectsMask = 0;

    // Negate Mean Look, Attract, and Bind flags
    for (i = 0; i < maxBattlers; i++) {
        if ((battleCtx->battleMons[i].statusVolatile & VOLATILE_CONDITION_MEAN_LOOK)
                && battleCtx->battleMons[i].moveEffectsData.meanLookTarget == battler) {
            battleCtx->battleMons[i].statusVolatile &= ~VOLATILE_CONDITION_MEAN_LOOK;
        }

        if (battleCtx->battleMons[i].statusVolatile & (FlagIndex(battler) << VOLATILE_CONDITION_ATTRACT_SHIFT)) {
            battleCtx->battleMons[i].statusVolatile &= (FlagIndex(battler) << VOLATILE_CONDITION_ATTRACT_SHIFT) ^ 0xFFFFFFFF;
        }

        if ((battleCtx->battleMons[i].statusVolatile & VOLATILE_CONDITION_BIND)
                && battleCtx->battleMons[i].moveEffectsData.bindTarget == battler) {
            battleCtx->battleMons[i].statusVolatile &= ~VOLATILE_CONDITION_BIND;
        }
    }

    u8 *addr = (u8 *)&battleCtx->battleMons[battler].moveEffectsData;
    for (i = 0; i < sizeof(MoveEffectsData); i++) {
        addr[i] = 0;
    }

    addr = (u8 *)&battleCtx->turnFlags[battler];
    for (i = 0; i < sizeof(struct TurnFlags); i++) {
        addr[i] = 0;
    }

    battleCtx->battleMons[battler].moveEffectsData.fakeOutTurnNumber = battleCtx->totalTurns + 1;
    battleCtx->battleMons[battler].moveEffectsData.slowStartTurnNumber = battleCtx->totalTurns + 1;
    battleCtx->battleMons[battler].moveEffectsData.truant = (battleCtx->totalTurns + 1) & 1;
    battleCtx->moveProtect[battler] = MOVE_NONE;
    battleCtx->moveHit[battler] = MOVE_NONE;
    battleCtx->moveHitBattler[battler] = BATTLER_NONE;
    battleCtx->moveHitType[battler] = MOVE_NONE;
    battleCtx->movePrevByBattler[battler] = MOVE_NONE;
    battleCtx->moveCopied[battler] = MOVE_NONE;
    battleCtx->moveCopiedHit[battler][0] = MOVE_NONE;
    battleCtx->moveCopiedHit[battler][1] = MOVE_NONE;
    battleCtx->moveCopiedHit[battler][2] = MOVE_NONE;
    battleCtx->moveCopiedHit[battler][3] = MOVE_NONE;
    battleCtx->moveSketched[battler] = MOVE_NONE;
    battleCtx->conversion2Move[battler] = MOVE_NONE;
    battleCtx->conversion2Battler[battler] = MOVE_NONE;
    battleCtx->conversion2Type[battler] = MOVE_NONE;
    battleCtx->metronomeMove[battler] = MOVE_NONE;
    battleCtx->fieldConditionsMask &= (FlagIndex(battler) << FIELD_CONDITION_UPROAR_SHIFT) ^ 0xFFFFFFFF;

    for (i = 0; i < maxBattlers; i++) {
        if (i != battler && Battler_Side(battleSys, i) != Battler_Side(battleSys, battler)) {
            battleCtx->moveCopied[i] = MOVE_NONE;
        }
        battleCtx->moveCopiedHit[i][battler] = 0;
    }

    battleCtx->battlerStatusFlags[battler] &= ~BATTLER_STATUS_SWITCHING;
    BattleAI_ClearKnownMoves(battleCtx, battler);
    BattleAI_ClearKnownAbility(battleCtx, battler);
    BattleAI_ClearKnownItem(battleCtx, battler);
}

void BattleSystem_SetupNextTurn (BattleSystem * param0, BattleContext * param1)
{
    int v0;

    for (v0 = 0; v0 < 4; v0++) {
        MI_CpuClearFast(&param1->turnFlags[v0], sizeof(struct TurnFlags));
        MI_CpuClearFast(&param1->moveFailFlags[v0], sizeof(struct MoveFailFlags));

        param1->battleMons[v0].statusVolatile &= (0x8 ^ 0xffffffff);

        if (param1->battleMons[v0].moveEffectsData.rechargeTurnNumber + 1 < param1->totalTurns) {
            param1->battleMons[v0].statusVolatile &= (0x400000 ^ 0xffffffff);
        }

        if ((param1->battleMons[v0].status & 0x7) && (param1->battleMons[v0].statusVolatile & 0x1000)) {
            Battler_UnlockMoveChoice(param0, param1, v0);
        }

        if ((param1->battleMons[v0].status & 0x7) && (param1->battleMons[v0].statusVolatile & 0xc00)) {
            param1->battleMons[v0].statusVolatile &= (0xc00 ^ 0xffffffff);
        }
    }

    param1->sideConditions[0].followMe = 0;
    param1->sideConditions[1].followMe = 0;
}

int BattleSystem_CheckStruggling (BattleSystem *battleSys, BattleContext *battleCtx, int battler, int moveFlags, int struggleChecksMask)
{
    int v0;
    int v1;

    v1 = Battler_HeldItemEffect(battleCtx, battler);

    for (v0 = 0; v0 < 4; v0++) {
        if ((battleCtx->battleMons[battler].moves[v0] == 0) && (struggleChecksMask & 0x1)) {
            moveFlags |= FlagIndex(v0);
        }

        if ((battleCtx->battleMons[battler].ppCur[v0] == 0) && (struggleChecksMask & 0x2)) {
            moveFlags |= FlagIndex(v0);
        }

        if ((battleCtx->battleMons[battler].moves[v0] == battleCtx->battleMons[battler].moveEffectsData.disabledMove) && (struggleChecksMask & 0x4)) {
            moveFlags |= FlagIndex(v0);
        }

        if ((battleCtx->battleMons[battler].moves[v0] == battleCtx->movePrevByBattler[battler]) && (struggleChecksMask & 0x8) && (battleCtx->battleMons[battler].statusVolatile & 0x80000000)) {
            moveFlags |= FlagIndex(v0);
        }

        if ((battleCtx->battleMons[battler].moveEffectsData.tauntedTurns) && (struggleChecksMask & 0x10) && (battleCtx->aiContext.moveTable[battleCtx->battleMons[battler].moves[v0]].power == 0)) {
            moveFlags |= FlagIndex(v0);
        }

        if ((BattleSystem_Imprisoned(battleSys, battleCtx, battler, battleCtx->battleMons[battler].moves[v0])) && (struggleChecksMask & 0x20)) {
            moveFlags |= FlagIndex(v0);
        }

        if ((BattleSystem_FailsInHighGravity(battleSys, battleCtx, battler, battleCtx->battleMons[battler].moves[v0])) && (struggleChecksMask & 0x40)) {
            moveFlags |= FlagIndex(v0);
        }

        if ((BattleSystem_HealBlocked(battleSys, battleCtx, battler, battleCtx->battleMons[battler].moves[v0])) && (struggleChecksMask & 0x80)) {
            moveFlags |= FlagIndex(v0);
        }

        if ((battleCtx->battleMons[battler].moveEffectsData.encoredMove) && (battleCtx->battleMons[battler].moveEffectsData.encoredMove != battleCtx->battleMons[battler].moves[v0])) {
            moveFlags |= FlagIndex(v0);
        }

        if (((v1 == 55) || (v1 == 115) || (v1 == 125)) && (struggleChecksMask & 0x200)) {
            if (Battler_SlotForMove(&battleCtx->battleMons[battler], battleCtx->battleMons[battler].moveEffectsData.choiceLockedMove) == 4) {
                battleCtx->battleMons[battler].moveEffectsData.choiceLockedMove = 0;
            } else {
                if ((battleCtx->battleMons[battler].moveEffectsData.choiceLockedMove) && (battleCtx->battleMons[battler].moveEffectsData.choiceLockedMove != battleCtx->battleMons[battler].moves[v0])) {
                    moveFlags |= FlagIndex(v0);
                }
            }
        }
    }

    return moveFlags;
}

BOOL BattleSystem_CanUseMove (BattleSystem *battleSys, BattleContext *battleCtx, int battler, int moveSlot, BattleMessage *msgOut)
{
    BOOL result = TRUE;

    if (BattleSystem_CheckStruggling(battleSys, battleCtx, battler, 0, 0x4) & FlagIndex(moveSlot)) {
        msgOut->tags = 10;
        msgOut->id = 609;
        msgOut->params[0] = BattleSystem_NicknameTag(battleCtx, battler);
        msgOut->params[1] = battleCtx->battleMons[battler].moves[moveSlot];
        result = FALSE;
    } else if (BattleSystem_CheckStruggling(battleSys, battleCtx, battler, 0, 0x8) & FlagIndex(moveSlot)) {
        msgOut->tags = 2;
        msgOut->id = 612;
        msgOut->params[0] = BattleSystem_NicknameTag(battleCtx, battler);
        result = FALSE;
    } else if (BattleSystem_CheckStruggling(battleSys, battleCtx, battler, 0, 0x10) & FlagIndex(moveSlot)) {
        msgOut->tags = 10;
        msgOut->id = 613;
        msgOut->params[0] = BattleSystem_NicknameTag(battleCtx, battler);
        msgOut->params[1] = battleCtx->battleMons[battler].moves[moveSlot];
        result = FALSE;
    } else if (BattleSystem_CheckStruggling(battleSys, battleCtx, battler, 0, 0x20) & FlagIndex(moveSlot)) {
        msgOut->tags = 10;
        msgOut->id = 616;
        msgOut->params[0] = BattleSystem_NicknameTag(battleCtx, battler);
        msgOut->params[1] = battleCtx->battleMons[battler].moves[moveSlot];
        result = FALSE;
    } else if (BattleSystem_CheckStruggling(battleSys, battleCtx, battler, 0, 0x40) & FlagIndex(moveSlot)) {
        msgOut->tags = 10;
        msgOut->id = 1001;
        msgOut->params[0] = BattleSystem_NicknameTag(battleCtx, battler);
        msgOut->params[1] = battleCtx->battleMons[battler].moves[moveSlot];
        result = FALSE;
    } else if (BattleSystem_CheckStruggling(battleSys, battleCtx, battler, 0, 0x80) & FlagIndex(moveSlot)) {
        msgOut->tags = 34;
        msgOut->id = 1057;
        msgOut->params[0] = BattleSystem_NicknameTag(battleCtx, battler);
        msgOut->params[1] = 377;
        msgOut->params[2] = battleCtx->battleMons[battler].moves[moveSlot];
        result = FALSE;
    } else if (BattleSystem_CheckStruggling(battleSys, battleCtx, battler, 0, 0x200) & FlagIndex(moveSlot)) {
        msgOut->tags = 24;
        msgOut->id = 911;
        msgOut->params[0] = battleCtx->battleMons[battler].heldItem;
        msgOut->params[1] = battleCtx->battleMons[battler].moveEffectsData.choiceLockedMove;
        result = FALSE;
    } else if (BattleSystem_CheckStruggling(battleSys, battleCtx, battler, 0, 0x2) & FlagIndex(moveSlot)) {
        msgOut->tags = 0;
        msgOut->id = 823;
        result = FALSE;
    }

    return result;
}

int Battler_SlotForMove (BattleMon * param0, u16 param1)
{
    int v0;

    for (v0 = 0; v0 < LEARNED_MOVES_MAX; v0++) {
        if (param0->moves[v0] == param1) {
            break;
        }
    }

    return v0;
}

/**
 * @brief The type matchup table. This lists the matchups which are deemed to
 * have a Super Effective, Not Very Effective, or Immune result. The table can
 * be read as such:
 * 
 *   attacking type, defending type, matchup multiplier
 * 
 * The matchup multiplier is assumed to be divided by 10 at a later stage for
 * any computations which make use of it (hence the values 5 and 20).
 */
static const u8 sTypeMatchupMultipliers[][3] = {
    { TYPE_NORMAL,   TYPE_ROCK,      TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_NORMAL,   TYPE_STEEL,     TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_FIRE,     TYPE_FIRE,      TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_FIRE,     TYPE_WATER,     TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_FIRE,     TYPE_GRASS,     TYPE_MULTI_SUPER_EFF    },
    { TYPE_FIRE,     TYPE_ICE,       TYPE_MULTI_SUPER_EFF    },
    { TYPE_FIRE,     TYPE_BUG,       TYPE_MULTI_SUPER_EFF    },
    { TYPE_FIRE,     TYPE_ROCK,      TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_FIRE,     TYPE_DRAGON,    TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_FIRE,     TYPE_STEEL,     TYPE_MULTI_SUPER_EFF    },
    { TYPE_WATER,    TYPE_FIRE,      TYPE_MULTI_SUPER_EFF    },
    { TYPE_WATER,    TYPE_WATER,     TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_WATER,    TYPE_GRASS,     TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_WATER,    TYPE_GROUND,    TYPE_MULTI_SUPER_EFF    },
    { TYPE_WATER,    TYPE_ROCK,      TYPE_MULTI_SUPER_EFF    },
    { TYPE_WATER,    TYPE_DRAGON,    TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_ELECTRIC, TYPE_WATER,     TYPE_MULTI_SUPER_EFF    },
    { TYPE_ELECTRIC, TYPE_ELECTRIC,  TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_ELECTRIC, TYPE_GRASS,     TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_ELECTRIC, TYPE_GROUND,    TYPE_MULTI_IMMUNE       },
    { TYPE_ELECTRIC, TYPE_FLYING,    TYPE_MULTI_SUPER_EFF    },
    { TYPE_ELECTRIC, TYPE_DRAGON,    TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_GRASS,    TYPE_FIRE,      TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_GRASS,    TYPE_WATER,     TYPE_MULTI_SUPER_EFF    },
    { TYPE_GRASS,    TYPE_GRASS,     TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_GRASS,    TYPE_POISON,    TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_GRASS,    TYPE_GROUND,    TYPE_MULTI_SUPER_EFF    },
    { TYPE_GRASS,    TYPE_FLYING,    TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_GRASS,    TYPE_BUG,       TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_GRASS,    TYPE_ROCK,      TYPE_MULTI_SUPER_EFF    },
    { TYPE_GRASS,    TYPE_DRAGON,    TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_GRASS,    TYPE_STEEL,     TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_ICE,      TYPE_WATER,     TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_ICE,      TYPE_GRASS,     TYPE_MULTI_SUPER_EFF    },
    { TYPE_ICE,      TYPE_ICE,       TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_ICE,      TYPE_GROUND,    TYPE_MULTI_SUPER_EFF    },
    { TYPE_ICE,      TYPE_FLYING,    TYPE_MULTI_SUPER_EFF    },
    { TYPE_ICE,      TYPE_DRAGON,    TYPE_MULTI_SUPER_EFF    },
    { TYPE_ICE,      TYPE_STEEL,     TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_ICE,      TYPE_FIRE,      TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_FIGHTING, TYPE_NORMAL,    TYPE_MULTI_SUPER_EFF    },
    { TYPE_FIGHTING, TYPE_ICE,       TYPE_MULTI_SUPER_EFF    },
    { TYPE_FIGHTING, TYPE_POISON,    TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_FIGHTING, TYPE_FLYING,    TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_FIGHTING, TYPE_PSYCHIC,   TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_FIGHTING, TYPE_BUG,       TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_FIGHTING, TYPE_ROCK,      TYPE_MULTI_SUPER_EFF    },
    { TYPE_FIGHTING, TYPE_DARK,      TYPE_MULTI_SUPER_EFF    },
    { TYPE_FIGHTING, TYPE_STEEL,     TYPE_MULTI_SUPER_EFF    },
    { TYPE_POISON,   TYPE_GRASS,     TYPE_MULTI_SUPER_EFF    },
    { TYPE_POISON,   TYPE_POISON,    TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_POISON,   TYPE_GROUND,    TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_POISON,   TYPE_ROCK,      TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_POISON,   TYPE_GHOST,     TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_POISON,   TYPE_STEEL,     TYPE_MULTI_IMMUNE       },
    { TYPE_GROUND,   TYPE_FIRE,      TYPE_MULTI_SUPER_EFF    },
    { TYPE_GROUND,   TYPE_ELECTRIC,  TYPE_MULTI_SUPER_EFF    },
    { TYPE_GROUND,   TYPE_GRASS,     TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_GROUND,   TYPE_POISON,    TYPE_MULTI_SUPER_EFF    },
    { TYPE_GROUND,   TYPE_FLYING,    TYPE_MULTI_IMMUNE       },
    { TYPE_GROUND,   TYPE_BUG,       TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_GROUND,   TYPE_ROCK,      TYPE_MULTI_SUPER_EFF    },
    { TYPE_GROUND,   TYPE_STEEL,     TYPE_MULTI_SUPER_EFF    },
    { TYPE_FLYING,   TYPE_ELECTRIC,  TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_FLYING,   TYPE_GRASS,     TYPE_MULTI_SUPER_EFF    },
    { TYPE_FLYING,   TYPE_FIGHTING,  TYPE_MULTI_SUPER_EFF    },
    { TYPE_FLYING,   TYPE_BUG,       TYPE_MULTI_SUPER_EFF    },
    { TYPE_FLYING,   TYPE_ROCK,      TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_FLYING,   TYPE_STEEL,     TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_PSYCHIC,  TYPE_FIGHTING,  TYPE_MULTI_SUPER_EFF    },
    { TYPE_PSYCHIC,  TYPE_POISON,    TYPE_MULTI_SUPER_EFF    },
    { TYPE_PSYCHIC,  TYPE_PSYCHIC,   TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_PSYCHIC,  TYPE_DARK,      TYPE_MULTI_IMMUNE       },
    { TYPE_PSYCHIC,  TYPE_STEEL,     TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_BUG,      TYPE_FIRE,      TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_BUG,      TYPE_GRASS,     TYPE_MULTI_SUPER_EFF    },
    { TYPE_BUG,      TYPE_FIGHTING,  TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_BUG,      TYPE_POISON,    TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_BUG,      TYPE_FLYING,    TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_BUG,      TYPE_PSYCHIC,   TYPE_MULTI_SUPER_EFF    },
    { TYPE_BUG,      TYPE_GHOST,     TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_BUG,      TYPE_DARK,      TYPE_MULTI_SUPER_EFF    },
    { TYPE_BUG,      TYPE_STEEL,     TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_ROCK,     TYPE_FIRE,      TYPE_MULTI_SUPER_EFF    },
    { TYPE_ROCK,     TYPE_ICE,       TYPE_MULTI_SUPER_EFF    },
    { TYPE_ROCK,     TYPE_FIGHTING,  TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_ROCK,     TYPE_GROUND,    TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_ROCK,     TYPE_FLYING,    TYPE_MULTI_SUPER_EFF    },
    { TYPE_ROCK,     TYPE_BUG,       TYPE_MULTI_SUPER_EFF    },
    { TYPE_ROCK,     TYPE_STEEL,     TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_GHOST,    TYPE_NORMAL,    TYPE_MULTI_IMMUNE       },
    { TYPE_GHOST,    TYPE_PSYCHIC,   TYPE_MULTI_SUPER_EFF    },
    { TYPE_GHOST,    TYPE_DARK,      TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_GHOST,    TYPE_STEEL,     TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_GHOST,    TYPE_GHOST,     TYPE_MULTI_SUPER_EFF    },
    { TYPE_DRAGON,   TYPE_DRAGON,    TYPE_MULTI_SUPER_EFF    },
    { TYPE_DRAGON,   TYPE_STEEL,     TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_DARK,     TYPE_FIGHTING,  TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_DARK,     TYPE_PSYCHIC,   TYPE_MULTI_SUPER_EFF    },
    { TYPE_DARK,     TYPE_GHOST,     TYPE_MULTI_SUPER_EFF    },
    { TYPE_DARK,     TYPE_DARK,      TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_DARK,     TYPE_STEEL,     TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_STEEL,    TYPE_FIRE,      TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_STEEL,    TYPE_WATER,     TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_STEEL,    TYPE_ELECTRIC,  TYPE_MULTI_NOT_VERY_EFF },
    { TYPE_STEEL,    TYPE_ICE,       TYPE_MULTI_SUPER_EFF    },
    { TYPE_STEEL,    TYPE_ROCK,      TYPE_MULTI_SUPER_EFF    },
    { TYPE_STEEL,    TYPE_STEEL,     TYPE_MULTI_NOT_VERY_EFF },

    { 0xFE, 0xFE, TYPE_MULTI_IMMUNE },

    // These values are separated from the remainder of the table to support
    // the Foresight effect, which removes these immunities from consideration.
    { TYPE_NORMAL,   TYPE_GHOST,    TYPE_MULTI_IMMUNE        },
    { TYPE_FIGHTING, TYPE_GHOST,    TYPE_MULTI_IMMUNE        },

    { 0xFF, 0xFF, TYPE_MULTI_IMMUNE },
};

static BOOL ov16_02254EF4 (BattleContext * param0, int param1, int param2, int param3)
{
    BOOL v0;
    int v1;

    v1 = Battler_HeldItemEffect(param0, param2);
    v0 = 1;

    if ((v1 == 106) || (param0->battleMons[param2].moveEffectsMask & 0x400)) {
        if ((sTypeMatchupMultipliers[param3][1] == 2) && (sTypeMatchupMultipliers[param3][2] == 0)) {
            v0 = 0;
        }
    }

    if (param0->turnFlags[param2].roosting) {
        if (sTypeMatchupMultipliers[param3][1] == 2) {
            v0 = 0;
        }
    }

    if (param0->fieldConditionsMask & 0x7000) {
        if ((sTypeMatchupMultipliers[param3][1] == 2) && (sTypeMatchupMultipliers[param3][2] == 0)) {
            v0 = 0;
        }
    }

    if (param0->battleMons[param2].moveEffectsMask & 0x400000) {
        if ((sTypeMatchupMultipliers[param3][1] == 17) && (sTypeMatchupMultipliers[param3][2] == 0)) {
            v0 = 0;
        }
    }

    return v0;
}

int BattleSystem_CheckTypeChart (BattleSystem * param0, BattleContext * param1, int param2, int param3, int param4, int param5, int param6, u32 * param7)
{
    int v0;
    int v1;
    u8 v2;
    u32 v3;
    u8 v4;
    u8 v5;
    u8 v6;
    u8 v7;

    v1 = 1;

    if (param2 == 165) {
        return param6;
    }

    v4 = Battler_HeldItemEffect(param1, param4);
    v6 = Battler_HeldItemPower(param1, param4, 0);
    v5 = Battler_HeldItemEffect(param1, param5);
    v7 = Battler_HeldItemPower(param1, param5, 0);

    if (Battler_Ability(param1, param4) == 96) {
        v2 = 0;
    } else if (param3) {
        v2 = param3;
    } else {
        v2 = param1->aiContext.moveTable[param2].type;
    }

    v3 = param1->aiContext.moveTable[param2].power;

    if (((param1->battleStatusMask & 0x800) == 0) && ((BattleMon_Get(param1, param4, 27, NULL) == v2) || (BattleMon_Get(param1, param4, 28, NULL) == v2))) {
        if (Battler_Ability(param1, param4) == 91) {
            param6 *= 2;
        } else {
            param6 = param6 * 15 / 10;
        }
    }

    if ((Battler_IgnorableAbility(param1, param4, param5, 26) == 1) && (v2 == 4) && (v5 != 106)) {
        param7[0] |= 0x800;
    } else if ((param1->battleMons[param5].moveEffectsData.magnetRiseTurns) && ((param1->battleMons[param5].moveEffectsMask & 0x400) == 0) && (v2 == 4) && (v5 != 106)) {
        param7[0] |= 0x100000;
    } else {
        v0 = 0;

        while (sTypeMatchupMultipliers[v0][0] != 0xff) {
            if (sTypeMatchupMultipliers[v0][0] == 0xfe) {
                if ((param1->battleMons[param5].statusVolatile & 0x20000000) || (Battler_Ability(param1, param4) == 113)) {
                    break;
                } else {
                    v0++;
                    continue;
                }
            }

            if (sTypeMatchupMultipliers[v0][0] == v2) {
                if (sTypeMatchupMultipliers[v0][1] == BattleMon_Get(param1, param5, 27, NULL)) {
                    if (ov16_02254EF4(param1, param4, param5, v0) == 1) {
                        param6 = ov16_0225B63C(param1, param4, sTypeMatchupMultipliers[v0][2], param6, v3, param7);

                        if (sTypeMatchupMultipliers[v0][2] == 20) {
                            v1 *= 2;
                        }
                    }
                }

                if ((sTypeMatchupMultipliers[v0][1] == BattleMon_Get(param1, param5, 28, NULL)) && (BattleMon_Get(param1, param5, 27, NULL) != BattleMon_Get(param1, param5, 28, NULL))) {
                    if (ov16_02254EF4(param1, param4, param5, v0) == 1) {
                        param6 = ov16_0225B63C(param1, param4, sTypeMatchupMultipliers[v0][2], param6, v3, param7);

                        if (sTypeMatchupMultipliers[v0][2] == 20) {
                            v1 *= 2;
                        }
                    }
                }
            }

            v0++;
        }
    }

    if ((Battler_IgnorableAbility(param1, param4, param5, 25) == 1) && (ov16_0225B6C8(param1, param2)) && (((param7[0] & 0x2) == 0) || ((param7[0] & (2 | 4)) == (2 | 4))) && (v3)) {
        param7[0] |= 0x40000;
    } else {
        if (((param1->battleStatusMask & 0x800) == 0) && ((param1->battleStatusMask & 0x8000) == 0)) {
            if ((param7[0] & 0x2) && (v3)) {
                if ((Battler_IgnorableAbility(param1, param4, param5, 111) == 1) || (Battler_IgnorableAbility(param1, param4, param5, 116) == 1)) {
                    param6 = BattleSystem_Divide(param6 * 3, 4);
                }

                if (v4 == 96) {
                    param6 = param6 * (100 + v6) / 100;
                }
            }

            if ((param7[0] & 0x4) && (v3)) {
                if (Battler_Ability(param1, param4) == 110) {
                    param6 *= 2;
                }
            }
        } else {
            param7[0] &= (0x2 ^ 0xffffffff);
            param7[0] &= (0x4 ^ 0xffffffff);
        }
    }

    return param6;
}

void ov16_022552D4 (BattleContext * param0, int param1, int param2, int param3, int param4, int param5, int param6, int param7, u32 * param8)
{
    int v0;
    u8 v1;

    if (param1 == 165) {
        return;
    }

    if (param3 == 96) {
        v1 = 0;
    } else if (param2) {
        v1 = param2;
    } else {
        v1 = param0->aiContext.moveTable[param1].type;
    }

    if ((param3 != 104) && (param4 == 26) && (v1 == 4) && ((param0->fieldConditionsMask & 0x7000) == 0) && (param5 != 106)) {
        param8[0] |= 0x8;
    } else {
        v0 = 0;

        while (sTypeMatchupMultipliers[v0][0] != 0xff) {
            if (sTypeMatchupMultipliers[v0][0] == 0xfe) {
                if (param3 == 113) {
                    break;
                } else {
                    v0++;
                    continue;
                }
            }

            if (sTypeMatchupMultipliers[v0][0] == v1) {
                if (sTypeMatchupMultipliers[v0][1] == param6) {
                    if (ov16_022553F8(param0, param5, v0) == 1) {
                        ov16_02255448(sTypeMatchupMultipliers[v0][2], param8);
                    }
                }

                if ((sTypeMatchupMultipliers[v0][1] == param7) && (param6 != param7)) {
                    if (ov16_022553F8(param0, param5, v0) == 1) {
                        ov16_02255448(sTypeMatchupMultipliers[v0][2], param8);
                    }
                }
            }

            v0++;
        }
    }

    if ((param3 != 104) && (param4 == 25) && (ov16_0225B6C8(param0, param1)) && (((param8[0] & 0x2) == 0) || ((param8[0] & (2 | 4)) == (2 | 4)))) {
        param8[0] |= 0x8;
    }

    return;
}

static BOOL ov16_022553F8 (BattleContext * param0, int param1, int param2)
{
    BOOL v0;

    v0 = 1;

    if (param1 == 106) {
        if ((sTypeMatchupMultipliers[param2][1] == 2) && (sTypeMatchupMultipliers[param2][2] == 0)) {
            v0 = 0;
        }
    }

    if (param0->fieldConditionsMask & 0x7000) {
        if ((sTypeMatchupMultipliers[param2][1] == 2) && (sTypeMatchupMultipliers[param2][2] == 0)) {
            v0 = 0;
        }
    }

    return v0;
}

static void ov16_02255448 (int param0, u32 * param1)
{
    switch (param0) {
    case 0:
        param1[0] |= 0x8;
        param1[0] &= (0x4 ^ 0xffffffff);
        param1[0] &= (0x2 ^ 0xffffffff);
        break;
    case 5:
        if (param1[0] & 0x2) {
            param1[0] &= (0x2 ^ 0xffffffff);
        } else {
            param1[0] |= 0x4;
        }
        break;
    case 20:
        if (param1[0] & 0x4) {
            param1[0] &= (0x4 ^ 0xffffffff);
        } else {
            param1[0] |= 0x2;
        }
        break;
    }

    return;
}

BOOL BattleContext_MoveFailed (BattleContext * param0, int param1)
{
    if ((param0->moveFailFlags[param1].paralyzed) || (param0->moveFailFlags[param1].noEffect) || (param0->moveFailFlags[param1].imprisoned) || (param0->moveFailFlags[param1].infatuated) || (param0->moveFailFlags[param1].disabled) || (param0->moveFailFlags[param1].taunted) || (param0->moveFailFlags[param1].flinched) || (param0->moveFailFlags[param1].gravity) || (param0->moveFailFlags[param1].confused)) {
        return 1;
    } else {
        return 0;
    }
}

u8 BattleSystem_CountAliveBattlers(BattleSystem *battleSys, BattleContext *battleCtx, BOOL sameSide, int defender)
{
    u8 count = 0;
    int maxBattlers = BattleSystem_MaxBattlers(battleSys);
    
    // no clue why they used a switch statement for this, but changing it to an if-else doesn't match
    switch (sameSide) {
    case FALSE:
        for (int i = 0; i < maxBattlers; i++) {
            if (i != defender && battleCtx->battleMons[i].curHP) {
                count++;
            }
        }
        break;

    case TRUE:
        for (int i = 0; i < maxBattlers; i++) {
            if (Battler_Side(battleSys, i) == Battler_Side(battleSys, defender)
                    && battleCtx->battleMons[i].curHP) {
                count++;
            }
        }
        break;
    }

    return count;
}

int BattleSystem_NicknameTag (BattleContext *battleSys, int battler)
{
    return battler | (battleSys->selectedPartySlot[battler] << 8);
}

u16 Battler_SelectedMove (BattleContext * param0, int param1)
{
    u16 v0;

    v0 = 0;

    if ((param0->battlerActions[param1][3] == 1) && (param0->battlerActions[param1][2])) {
        v0 = param0->battleMons[param1].moves[param0->battlerActions[param1][2] - 1];
    }

    return v0;
}

int BattleSystem_CountAbility(BattleSystem *battleSys, BattleContext *battleCtx, enum CountAbilityMode mode, int battler, int ability)
{
    int result = 0;
    int i;
    int maxBattlers = BattleSystem_MaxBattlers(battleSys);

    switch (mode) {
    case COUNT_ALL_BATTLERS_OUR_SIDE:
        for (i = 0; i < maxBattlers; i++) {
            if (Battler_Side(battleSys, i) == Battler_Side(battleSys, battler)
                    && Battler_Ability(battleCtx, i) == ability) {
                result++;
            }
        }
        break;
    
    case COUNT_ALIVE_BATTLERS_OUR_SIDE:
        for (i = 0; i < maxBattlers; i++) {
            if (Battler_Side(battleSys, i) == Battler_Side(battleSys, battler)
                    && battleCtx->battleMons[i].curHP
                    && Battler_Ability(battleCtx, i) == ability) {
                result++;
            }
        }
        break;

    case COUNT_ALL_BATTLERS_THEIR_SIDE:
        for (i = 0; i < maxBattlers; i++) {
            if (Battler_Side(battleSys, i) != Battler_Side(battleSys, battler)
                    && Battler_Ability(battleCtx, i) == ability) {
                result++;
            }
        }
        break;

    case COUNT_ALIVE_BATTLERS_THEIR_SIDE:
        for (i = 0; i < maxBattlers; i++) {
            if (Battler_Side(battleSys, i) != Battler_Side(battleSys, battler)
                    && battleCtx->battleMons[i].curHP
                    && Battler_Ability(battleCtx, i) == ability) {
                result++;
            }
        }
        break;

    case COUNT_ALIVE_BATTLERS_THEIR_SIDE_FLAG:
        for (i = 0; i < maxBattlers; i++) {
            if (Battler_Side(battleSys, i) != Battler_Side(battleSys, battler)
                    && battleCtx->battleMons[i].curHP
                    && Battler_Ability(battleCtx, i) == ability) {
                result |= FlagIndex(i);
            }
        }
        break;

    case COUNT_ALL_BATTLERS:
        for (i = 0; i < maxBattlers; i++) {
            if (Battler_Ability(battleCtx, i) == ability) {
                result++;
            }
        }
        break;

    case COUNT_ALL_BATTLERS_EXCEPT_ME:
        for (i = 0; i < maxBattlers; i++) {
            if (i != battler
                    && Battler_Ability(battleCtx, i) == ability) {
                result++;
            }
        }
        break;

    case COUNT_ALL_BATTLERS_EXCEPT_ME_RETURN_BATTLER:
        for (i = 0; i < maxBattlers; i++) {
            if (i != battler
                    && Battler_Ability(battleCtx, i) == ability) {
                result = i + 1;
                break;
            }
        }
        break;

    case COUNT_ALIVE_BATTLERS:
        for (i = 0; i < maxBattlers; i++) {
            if (Battler_Ability(battleCtx, i) == ability
                    && battleCtx->battleMons[i].curHP) {
                result++;
            }
        }
        break;

    case COUNT_ALIVE_BATTLERS_EXCEPT_ME:
        for (i = 0; i < maxBattlers; i++) {
            if (i != battler
                    && Battler_Ability(battleCtx, i) == ability
                    && battleCtx->battleMons[i].curHP) {
                result++;
            }
        }
        break;
    }

    return result;
}

BOOL BattleMove_IsMultiTurn (BattleContext * param0, int param1)
{
    switch (param0->aiContext.moveTable[param1].effect) {
    case 26:
    case 39:
    case 75:
    case 145:
    case 151:
    case 155:
    case 255:
    case 256:
    case 263:
    case 272:
        return 1;
        break;
    }

    return 0;
}

BOOL BattleSystem_TypeMatchup(BattleSystem *battleSys, int idx, u8 *moveType, u8 *vsType, u8 *multi)
{
    BOOL result = TRUE;
    if (idx >= NELEMS(sTypeMatchupMultipliers)) {
        idx = BattleSystem_RandNext(battleSys) % NELEMS(sTypeMatchupMultipliers);
        result = FALSE;
    }

    *moveType = sTypeMatchupMultipliers[idx][0];
    *vsType = sTypeMatchupMultipliers[idx][1];
    *multi = sTypeMatchupMultipliers[idx][2];

    return result;
}

int BattleSystem_TypeMatchupMultiplier(u8 attackingType, u8 defendingType1, u8 defendingType2)
{
    int i = 0;
    int mul = 40;

    while (sTypeMatchupMultipliers[i][0] != 0xFF) {
        if (sTypeMatchupMultipliers[i][0] == attackingType) {
            if (sTypeMatchupMultipliers[i][1] == defendingType1) {
                mul = mul * sTypeMatchupMultipliers[i][2] / 10;
            }

            if (sTypeMatchupMultipliers[i][1] == defendingType2
                    && defendingType1 != defendingType2) {
                mul = mul * sTypeMatchupMultipliers[i][2] / 10;
            }
        }

        i++;
    }

    return mul;
}

BOOL Move_IsInvoker(u16 move)
{
    // declaring this as a single return didn't match
    if (move == MOVE_NONE
            || move == MOVE_SLEEP_TALK
            || move == MOVE_COPYCAT
            || move == MOVE_ASSIST
            || move == MOVE_ME_FIRST
            || move == MOVE_MIRROR_MOVE
            || move == MOVE_METRONOME) {
        return TRUE;
    }

    return FALSE;
}

BOOL BattleSystem_IsGhostCurse (BattleContext * param0, u16 param1, int param2)
{
    return (param1 == 174) && ((BattleMon_Get(param0, param2, 27, NULL) == 7) || (BattleMon_Get(param0, param2, 28, NULL) == 7));
}

BOOL BattleSystem_CanStealItem(BattleSystem *battleSys, BattleContext *battleCtx, int battler)
{
    BOOL result = FALSE;
    int side = Battler_Side(battleSys, battler);

    if (battleCtx->battleMons[battler].heldItem
            && (battleCtx->sideConditions[side].knockedOffItemsMask & FlagIndex(battleCtx->selectedPartySlot[battler])) == FALSE
            && Item_IsMail(battleCtx->battleMons[battler].heldItem) == FALSE) {
        result = TRUE;
    }

    return result;
}

BOOL BattleSystem_NotHoldingMail (BattleContext * param0, int param1)
{
    return Item_IsMail(param0->battleMons[param1].heldItem) == 0;
}

BOOL BattleSystem_CanWhirlwind(BattleSystem *battleSys, BattleContext *battleCtx)
{
    BOOL result = FALSE;

    if (ATTACKING_MON.level >= DEFENDING_MON.level) {
        result = TRUE;
    } else {
        int tmp = (((BattleSystem_RandNext(battleSys) & 0xFF) * (ATTACKING_MON.level + DEFENDING_MON.level)) >> 8) + 1;
        if (tmp > battleCtx->battleMons[battleCtx->defender].level / 4) {
            result = TRUE;
        }
    }

    return result;
}

u8 Battler_Ability (BattleContext * param0, int param1)
{
    if ((param0->battleMons[param1].moveEffectsMask & 0x200000) && (param0->battleMons[param1].ability != 121)) {
        return 0;
    } else if ((param0->fieldConditionsMask & 0x7000) && (param0->battleMons[param1].ability == 26)) {
        return 0;
    } else if ((param0->battleMons[param1].moveEffectsMask & 0x400) && (param0->battleMons[param1].ability == 26)) {
        return 0;
    } else {
        return param0->battleMons[param1].ability;
    }
}

BOOL Battler_IgnorableAbility(BattleContext *battleCtx, int attacker, int defender, int ability)
{
    BOOL result = FALSE;

    if (Battler_Ability(battleCtx, attacker) != ABILITY_MOLD_BREAKER) {
        if (Battler_Ability(battleCtx, defender) == ability) {
            result = TRUE;
        }
    } else if (Battler_Ability(battleCtx, defender) == ability
            && battleCtx->selfTurnFlags[attacker].moldBreakerActivated == FALSE) {
        battleCtx->selfTurnFlags[attacker].moldBreakerActivated = TRUE;
        battleCtx->battleStatusMask |= SYSCTL_APPLY_MOLD_BREAKER;
    }

    return result;
}

BOOL BattleSystem_AnyReplacementMons(BattleSystem *battleSys, BattleContext *battleCtx, int battler)
{
    // Declarations here are done C89-style to match.
    BOOL result;
    Party *party;
    Pokemon *pokemon;
    int partySize;
    int aliveMons = 0, neededAliveMons;
    int start, end;
    int selectedSlot1, selectedSlot2;
    u32 battleType;

    result = FALSE;
    battleType = BattleSystem_BattleType(battleSys);
    party = BattleSystem_Party(battleSys, battler);
    partySize = BattleSystem_PartyCount(battleSys, battler);

    if ((battleType & BATTLE_TYPE_2vs2)
            || ((battleType & BATTLE_TYPE_TAG)
                && (BattleSystem_BattlerSlot(battleSys, battler) & BATTLER_THEM))) {
        // have to copy these 4 identical assignments across each branch to match
        start = 0;
        end = partySize;
        neededAliveMons = 1;
        selectedSlot1 = battleCtx->selectedPartySlot[battler];
        selectedSlot2 = battleCtx->selectedPartySlot[battler];
    } else if (battleType & BATTLE_TYPE_DOUBLES) {
        start = 0;
        end = partySize;
        neededAliveMons = 1;
        selectedSlot1 = battleCtx->selectedPartySlot[battler];
        selectedSlot2 = battleCtx->selectedPartySlot[BattleSystem_Partner(battleSys, battler)];
    } else {
        start = 0;
        end = partySize;
        neededAliveMons = 1;
        selectedSlot1 = battleCtx->selectedPartySlot[battler];
        selectedSlot2 = battleCtx->selectedPartySlot[battler];
    }

    for (int i = start; i < end; i++) {
        pokemon = Party_GetPokemonBySlotIndex(party, i);
        if (Pokemon_GetValue(pokemon, MON_DATA_SPECIES, NULL)
                && Pokemon_GetValue(pokemon, MON_DATA_IS_EGG, NULL) == FALSE
                && Pokemon_GetValue(pokemon, MON_DATA_CURRENT_HP, NULL)
                && selectedSlot1 != i
                && selectedSlot2 != i) {
            aliveMons++;
        }
    }

    if (aliveMons >= neededAliveMons) {
        result = TRUE;
    }

    return result;
}

BOOL BattleSystem_Trapped (BattleSystem * param0, BattleContext * param1, int param2, BattleMessage * param3)
{
    int v0;
    int v1;
    u8 v2;
    int v3;
    u32 v4;

    v4 = BattleSystem_BattleType(param0);
    v3 = Battler_HeldItemEffect(param1, param2);

    if ((v3 == 63) || (v4 & (0x4 | 0x20 | 0x200 | 0x80)) || (Battler_Ability(param1, param2) == 50)) {
        return 0;
    }

    v2 = Battler_Side(param0, param2);
    v1 = BattleSystem_MaxBattlers(param0);

    if ((v0 = BattleSystem_CountAbility(param0, param1, 9, param2, 23)) && (Battler_Ability(param1, param2) != 23)) {
        if (param3 == NULL) {
            return 1;
        }

        param3->tags = 11;
        param3->id = 39;
        param3->params[0] = BattleSystem_NicknameTag(param1, v0);
        param3->params[1] = 23;
        return 1;
    }

    if ((v0 = BattleSystem_CountAbility(param0, param1, 3, param2, 71))) {
        if (((param1->fieldConditionsMask & 0x7000) == 0) && (v3 != 106)) {
            if ((Battler_Ability(param1, param2) != 26) && (param1->battleMons[param2].moveEffectsData.magnetRiseTurns == 0) && ((BattleMon_Get(param1, param2, 27, NULL) != 2) && (BattleMon_Get(param1, param2, 28, NULL) != 2))) {
                if (param3 == NULL) {
                    return 1;
                }

                param3->tags = 11;
                param3->id = 39;
                param3->params[0] = BattleSystem_NicknameTag(param1, v0);
                param3->params[1] = 71;
                return 1;
            }
        } else {
            if (param3 == NULL) {
                return 1;
            }

            param3->tags = 11;
            param3->id = 39;
            param3->params[0] = BattleSystem_NicknameTag(param1, v0);
            param3->params[1] = 71;
            return 1;
        }
    }

    if ((v0 = BattleSystem_CountAbility(param0, param1, 3, param2, 42)) && ((BattleMon_Get(param1, param2, 27, NULL) == 8) || (BattleMon_Get(param1, param2, 28, NULL) == 8))) {
        if (param3 == NULL) {
            return 1;
        }

        param3->tags = 11;
        param3->id = 39;
        param3->params[0] = BattleSystem_NicknameTag(param1, v0);
        param3->params[1] = 42;
        return 1;
    }

    if ((param1->battleMons[param2].statusVolatile & (0xe000 | 0x4000000)) || (param1->battleMons[param2].moveEffectsMask & 0x400)) {
        if (param3 == NULL) {
            return 1;
        }

        param3->tags = 0;
        param3->id = 794;
        return 1;
    }

    return 0;
}

BOOL BattleSystem_TryEscape (BattleSystem * param0, BattleContext * param1, int param2)
{
    BOOL v0;
    u8 v1;
    int v2;
    u8 v3;
    u32 v4;

    v4 = BattleSystem_BattleType(param0);
    v2 = Battler_HeldItemEffect(param1, param2);
    v0 = 0;

    if (v2 == 63) {
        param1->turnFlags[param2].fleeing = 1;
        v0 = 1;
    } else if (v4 & (0x4 | 0x20 | 0x200 | 0x80)) {
        v0 = 1;
    } else if (Battler_Ability(param1, param2) == 50) {
        param1->turnFlags[param2].fleeing = 2;
        v0 = 1;
    } else {
        if (param1->battleMons[param2].speed < param1->battleMons[param2 ^ 1].speed) {
            v1 = param1->battleMons[param2].speed * 128 / param1->battleMons[param2 ^ 1].speed + param1->runAttempts * 30;

            if (v1 > (BattleSystem_RandNext(param0) % 256)) {
                v0 = 1;
            }
        } else {
            v0 = 1;
        }

        if (v0 == 0) {
            BattleIO_IncrementRecord(param0, param2, 0, (((70 + 1)) + 27));
        }

        param1->runAttempts++;
    }

    return v0;
}

BOOL Battler_CheckTruant (BattleContext * param0, int param1)
{
    BOOL v0;

    v0 = 0;

    if (Battler_Ability(param0, param1) == 54) {
        if (param0->battleMons[param1].moveEffectsData.truant != (param0->totalTurns & 1)) {
            v0 = 1;
        }
    }

    return v0;
}

BOOL BattleSystem_Imprisoned (BattleSystem * param0, BattleContext * param1, int param2, int param3)
{
    int v0;
    int v1;
    int v2;
    int v3;
    BOOL v4;

    v4 = 0;
    v1 = BattleSystem_MaxBattlers(param0);
    v2 = Battler_Side(param0, param2);

    for (v0 = 0; v0 < v1; v0++) {
        if ((v2 != Battler_Side(param0, v0)) && (param1->battleMons[v0].moveEffectsMask & 0x2000)) {
            for (v3 = 0; v3 < 4; v3++) {
                if (param3 == param1->battleMons[v0].moves[v3]) {
                    break;
                }
            }

            if (v3 != 4) {
                v4 = 1;
            }
        }
    }

    return v4;
}

BOOL BattleSystem_AnyBattlersWithMoveEffect(BattleSystem *battleSys, BattleContext *battleCtx, int effectMask)
{
    BOOL result = FALSE;
    int maxBattlers = BattleSystem_MaxBattlers(battleSys);

    for (int i = 0; i < maxBattlers; i++) {
        if (battleCtx->battleMons[i].moveEffectsMask & effectMask) {
            result = TRUE;
            break;
        }
    }

    return result;
}

void BattleSystem_SetupLoop(BattleSystem *battleSys, BattleContext *battleCtx)
{
    battleCtx->moveStatusFlags = 0;
    battleCtx->criticalMul = 1;
    battleCtx->battleStatusMask &= ~SYSCTL_REUSE_LAST_MOVE;
}

void BattleSystem_SortMonsBySpeed (BattleSystem *battleSys, BattleContext *battleCtx)
{
    int v0;
    int v1;
    int v2, v3;
    int v4, v5;

    v1 = BattleSystem_MaxBattlers(battleSys);

    for (v0 = 0; v0 < v1; v0++) {
        battleCtx->monSpeedOrder[v0] = v0;
    }

    for (v2 = 0; v2 < v1 - 1; v2++) {
        for (v3 = v2 + 1; v3 < v1; v3++) {
            v4 = battleCtx->monSpeedOrder[v2];
            v5 = battleCtx->monSpeedOrder[v3];

            if (BattleSystem_CompareBattlerSpeed(battleSys, battleCtx, v4, v5, 1)) {
                battleCtx->monSpeedOrder[v2] = v5;
                battleCtx->monSpeedOrder[v3] = v4;
            }
        }
    }
}

static const u16 Unk_ov16_0226EBD4[] = {
    0x13,
    0x154,
    0x1A,
    0x88,
    0x96,
    0x189
};

BOOL BattleSystem_FailsInHighGravity (BattleSystem * param0, BattleContext * param1, int param2, int param3)
{
    int v0;
    BOOL v1;

    v1 = 0;

    if (param1->fieldConditionsMask & 0x7000) {
        for (v0 = 0; v0 < NELEMS(Unk_ov16_0226EBD4); v0++) {
            if (Unk_ov16_0226EBD4[v0] == param3) {
                v1 = 1;
                break;
            }
        }
    }

    return v1;
}

static const u16 Unk_ov16_0226EBFA[] = {
    0x69,
    0x87,
    0x9C,
    0xD0,
    0xEA,
    0xEB,
    0xEC,
    0x100,
    0x1C8,
    0x12F,
    0x163,
    0x1CD,
    0x169,
    0x111
};

BOOL BattleSystem_HealBlocked (BattleSystem * param0, BattleContext * param1, int param2, int param3)
{
    int v0;
    BOOL v1;

    v1 = 0;

    if (param1->battleMons[param2].moveEffectsData.healBlockTurns) {
        for (v0 = 0; v0 < NELEMS(Unk_ov16_0226EBFA); v0++) {
            if (Unk_ov16_0226EBFA[v0] == param3) {
                v1 = 1;
                break;
            }
        }
    }

    return v1;
}

void BattleSystem_UpdateLastResort (BattleSystem * param0, BattleContext * param1)
{
    int v0;

    if ((param1->moveTemp == 387) || (param1->battleMons[param1->attacker].moveEffectsData.lastResortCount == 4)) {
        return;
    }

    for (v0 = 0; v0 < param1->battleMons[param1->attacker].moveEffectsData.lastResortCount; v0++) {
        if (param1->battleMons[param1->attacker].moveEffectsData.lastResortMoves[v0] == param1->moveTemp) {
            return;
        }
    }

    param1->battleMons[param1->attacker].moveEffectsData.lastResortMoves[v0] = param1->moveTemp;
    param1->battleMons[param1->attacker].moveEffectsData.lastResortCount++;
}

int Battler_CountMoves(BattleSystem *battleSys, BattleContext *battleCtx, int battler)
{
    int i;
    for (i = 0; i < LEARNED_MOVES_MAX; i++) {
        if (battleCtx->battleMons[battler].moves[i] == MOVE_NONE) {
            break;
        }
    }

    return i;
}

static u16 sSoundMoves[] = {
    MOVE_GROWL,
    MOVE_ROAR,
    MOVE_SING,
    MOVE_SUPERSONIC,
    MOVE_SCREECH,
    MOVE_SNORE,
    MOVE_UPROAR,
    MOVE_METAL_SOUND,
    MOVE_GRASS_WHISTLE,
    MOVE_HYPER_VOICE,
    MOVE_BUG_BUZZ,
    MOVE_CHATTER,
};

int BattleSystem_TriggerImmunityAbility(BattleContext *battleCtx, int attacker, int defender)
{
    int subscript = NULL, moveType;

    if (Battler_Ability(battleCtx, attacker) == ABILITY_NORMALIZE) {
        moveType = TYPE_NORMAL;
    } else if (battleCtx->moveType) {
        moveType = battleCtx->moveType;
    } else {
        moveType = CURRENT_MOVE_DATA.type;
    }

    if (Battler_IgnorableAbility(battleCtx, attacker, defender, ABILITY_VOLT_ABSORB) == TRUE
            && moveType == TYPE_ELECTRIC
            && attacker != defender) {
        battleCtx->hpCalcTemp = BattleSystem_Divide(battleCtx->battleMons[defender].maxHP, 4);
        subscript = BATTLE_SUBSEQ_ABILITY_RESTORES_HP;
    }

    if (Battler_IgnorableAbility(battleCtx, attacker, defender, ABILITY_WATER_ABSORB) == TRUE
            && moveType == TYPE_WATER
            && (battleCtx->battleStatusMask & SYSCTL_FIRST_OF_MULTI_TURN) == FALSE // do not proc on first turn of Dive
            && CURRENT_MOVE_DATA.power) {
        battleCtx->hpCalcTemp = BattleSystem_Divide(battleCtx->battleMons[defender].maxHP, 4);
        subscript = BATTLE_SUBSEQ_ABILITY_RESTORES_HP;
    }

    if (Battler_IgnorableAbility(battleCtx, attacker, defender, ABILITY_FLASH_FIRE) == TRUE
            && moveType == TYPE_FIRE
            && (battleCtx->battleMons[defender].status & MON_CONDITION_FREEZE) == FALSE
            && (battleCtx->battleStatusMask & SYSCTL_FIRST_OF_MULTI_TURN) == FALSE
            && (CURRENT_MOVE_DATA.power || battleCtx->moveCur == MOVE_WILL_O_WISP)) {
        subscript = BATTLE_SUBSEQ_ABSORB_AND_BOOST_FIRE_TYPE_MOVES;
    }

    if (Battler_IgnorableAbility(battleCtx, attacker, defender, ABILITY_SOUNDPROOF) == TRUE) {
        for (int i = 0; i < NELEMS(sSoundMoves); i++) {
            if (sSoundMoves[i] == battleCtx->moveCur) {
                subscript = BATTLE_SUBSEQ_BLOCKED_BY_SOUNDPROOF;
                break;
            }
        }
    }

    if (Battler_IgnorableAbility(battleCtx, attacker, defender, ABILITY_MOTOR_DRIVE) == TRUE
            && moveType == TYPE_ELECTRIC
            && attacker != defender) {
        subscript = BATTLE_SUBSEQ_ABSORB_AND_SPEED_UP_1_STAGE;
    }

    if (Battler_IgnorableAbility(battleCtx, attacker, defender, ABILITY_DRY_SKIN) == TRUE
            && moveType == TYPE_WATER
            && (battleCtx->battleStatusMask & SYSCTL_FIRST_OF_MULTI_TURN) == FALSE
            && CURRENT_MOVE_DATA.power) {
        battleCtx->hpCalcTemp = BattleSystem_Divide(battleCtx->battleMons[defender].maxHP, 4);
        subscript = BATTLE_SUBSEQ_ABILITY_RESTORES_HP;
    }

    return subscript;
}

BOOL BattleSystem_TriggerTurnEndAbility(BattleSystem *battleSys, BattleContext *battleCtx, int battler)
{
    BOOL result = FALSE;
    int subscript;

    switch (Battler_Ability(battleCtx, battler)) {
    case ABILITY_SPEED_BOOST:
        if (battleCtx->battleMons[battler].curHP
                && battleCtx->battleMons[battler].statBoosts[BATTLE_STAT_SPEED] < 12
                && battleCtx->battleMons[battler].moveEffectsData.fakeOutTurnNumber != battleCtx->totalTurns + 1) {
            battleCtx->sideEffectParam = MOVE_SUBSCRIPT_PTR_SPEED_UP_1_STAGE;
            battleCtx->sideEffectType = SIDE_EFFECT_TYPE_ABILITY;
            battleCtx->sideEffectMon = battler;
            subscript = BATTLE_SUBSEQ_UPDATE_STAT_STAGE;
            result = TRUE;
        }
        break;

    case ABILITY_SHED_SKIN:
        if ((battleCtx->battleMons[battler].status & MON_CONDITION_ANY)
                && battleCtx->battleMons[battler].curHP
                && BattleSystem_RandNext(battleSys) % 10 < 3) {
            if (battleCtx->battleMons[battler].status & MON_CONDITION_SLEEP) {
                battleCtx->msgTemp = MSGCOND_SLEEP;
            } else if (battleCtx->battleMons[battler].status & MON_CONDITION_ANY_POISON) {
                battleCtx->msgTemp = MSGCOND_POISON;
            } else if (battleCtx->battleMons[battler].status & MON_CONDITION_BURN) {
                battleCtx->msgTemp = MSGCOND_BURN;
            } else if (battleCtx->battleMons[battler].status & MON_CONDITION_PARALYSIS) {
                battleCtx->msgTemp = MSGCOND_PARALYSIS;
            } else {
                battleCtx->msgTemp = MSGCOND_FREEZE;
            }

            battleCtx->msgBattlerTemp = battler;
            subscript = BATTLE_SUBSEQ_ABILITY_RESTORE_STATUS;
            result = TRUE;
        }
        break;
    }

    if (result == TRUE) {
        LOAD_SUBSEQ(subscript);
        battleCtx->commandNext = battleCtx->command;
        battleCtx->command = BATTLE_CONTROL_EXEC_SCRIPT;
    }

    return result;
}

int BattleSystem_Divide(int dividend, int divisor)
{
    if (dividend == 0) {
        return dividend;
    }

    int signedFloor;
    if (dividend < 0) {
        signedFloor = -1;
    } else {
        signedFloor = 1;
    }

    dividend /= divisor;

    if (dividend == 0) {
        dividend = signedFloor;
    }

    return dividend;
}

enum {
    SWITCH_IN_CHECK_STATE_START = 0,

    SWITCH_IN_CHECK_STATE_FIELD_WEATHER = SWITCH_IN_CHECK_STATE_START,
    SWITCH_IN_CHECK_STATE_TRACE,
    SWITCH_IN_CHECK_STATE_WEATHER_ABILITIES,
    SWITCH_IN_CHECK_STATE_INTIMIDATE,
    SWITCH_IN_CHECK_STATE_DOWNLOAD,
    SWITCH_IN_CHECK_STATE_ANTICIPATION,
    SWITCH_IN_CHECK_STATE_FOREWARN,
    SWITCH_IN_CHECK_STATE_FRISK,
    SWITCH_IN_CHECK_STATE_SLOW_START,
    SWITCH_IN_CHECK_STATE_MOLD_BREAKER,
    SWITCH_IN_CHECK_STATE_PRESSURE,
    SWITCH_IN_CHECK_STATE_FORM_CHANGE,
    SWITCH_IN_CHECK_STATE_AMULET_COIN,
    SWITCH_IN_CHECK_STATE_FORBIDDEN_STATUS,
    SWITCH_IN_CHECK_STATE_HELD_ITEM_STATUS,

    SWITCH_IN_CHECK_STATE_DONE,
};

enum {
    SWITCH_IN_CHECK_RESULT_CONTINUE = 0,
    SWITCH_IN_CHECK_RESULT_BREAK,
    SWITCH_IN_CHECK_RESULT_DONE,
};

int BattleSystem_TriggerEffectOnSwitch(BattleSystem *battleSys, BattleContext *battleCtx)
{
    // must declare C89-style to match
    int i;
    int subscript;
    int result;
    int battler;
    int maxBattlers;

    maxBattlers = BattleSystem_MaxBattlers(battleSys);
    subscript = NULL;
    result = SWITCH_IN_CHECK_RESULT_CONTINUE;

    do {
        switch (battleCtx->switchInCheckState) {
        case SWITCH_IN_CHECK_STATE_FIELD_WEATHER:
            if (battleCtx->fieldWeatherChecked == FALSE) {
                switch (BattleSystem_FieldWeather(battleSys)) {
                case OVERWORLD_WEATHER_RAINING:
                case OVERWORLD_WEATHER_HEAVY_RAIN:
                case OVERWORLD_WEATHER_THUNDERSTORM:
                    subscript = BATTLE_SUBSEQ_OVERWORLD_RAIN;
                    result = SWITCH_IN_CHECK_RESULT_BREAK;
                    break;

                case OVERWORLD_WEATHER_SNOWING:
                case OVERWORLD_WEATHER_HEAVY_SNOW:
                case OVERWORLD_WEATHER_BLIZZARD:
                    subscript = BATTLE_SUBSEQ_OVERWORLD_HAIL;
                    result = SWITCH_IN_CHECK_RESULT_BREAK;
                    break;

                case OVERWORLD_WEATHER_SANDSTORM:
                    subscript = BATTLE_SUBSEQ_OVERWORLD_SAND;
                    result = SWITCH_IN_CHECK_RESULT_BREAK;
                    break;

                case OVERWORLD_WEATHER_FOG:
                case OVERWORLD_WEATHER_DEEP_FOG:
                    subscript = BATTLE_SUBSEQ_OVERWORLD_FOG;
                    result = SWITCH_IN_CHECK_RESULT_BREAK;
                    break;

                case OVERWORLD_WEATHER_HARSH_SUN:
                    subscript = BATTLE_SUBSEQ_OVERWORLD_SUN;
                    result = SWITCH_IN_CHECK_RESULT_BREAK;
                    break;

                case OVERWORLD_WEATHER_TRICK_ROOM:
                    subscript = BATTLE_SUBSEQ_OVERWORLD_TRICK_ROOM;
                    result = SWITCH_IN_CHECK_RESULT_BREAK;
                    break;

                default:
                    break;
                }

                if (result == SWITCH_IN_CHECK_RESULT_BREAK) {
                    battleCtx->fieldWeatherChecked = TRUE;
                }
            }

            battleCtx->switchInCheckState++;
            break;

        case SWITCH_IN_CHECK_STATE_TRACE:
            for (i = 0; i < maxBattlers; i++) {
                battler = battleCtx->monSpeedOrder[i];
                int defender1 = BattleSystem_EnemyInSlot(battleSys, battler, ENEMY_IN_SLOT_RIGHT);
                int defender2 = BattleSystem_EnemyInSlot(battleSys, battler, ENEMY_IN_SLOT_LEFT);

                battleCtx->msgDefender = ChooseTraceTarget(battleSys, battleCtx, defender1, defender2);

                if (battleCtx->battleMons[battler].traceAnnounced == FALSE
                        && battleCtx->msgDefender != BATTLER_NONE
                        && battleCtx->battleMons[battler].curHP
                        && battleCtx->battleMons[battler].heldItem != ITEM_GRISEOUS_ORB
                        && battleCtx->battleMons[battleCtx->msgDefender].curHP
                        && Battler_Ability(battleCtx, battler) == ABILITY_TRACE) {
                    battleCtx->battleMons[battler].traceAnnounced = TRUE;
                    battleCtx->msgBattlerTemp = battler;
                    subscript = BATTLE_SUBSEQ_TRACE;
                    result = SWITCH_IN_CHECK_RESULT_BREAK;
                    break;
                }
            }

            if (i == maxBattlers) {
                battleCtx->switchInCheckState++;
            }
            break;

        case SWITCH_IN_CHECK_STATE_WEATHER_ABILITIES:
            for (i = 0; i < maxBattlers; i++) {
                battler = battleCtx->monSpeedOrder[i];

                if (battleCtx->battleMons[battler].weatherAbilityAnnounced == FALSE
                        && battleCtx->battleMons[battler].curHP) {
                    switch (Battler_Ability(battleCtx, battler)) {
                    case ABILITY_DRIZZLE:
                        battleCtx->battleMons[battler].weatherAbilityAnnounced = TRUE;

                        if ((battleCtx->fieldConditionsMask & FIELD_CONDITION_RAINING_PERM) == FALSE) {
                            subscript = BATTLE_SUBSEQ_DRIZZLE;
                            result = SWITCH_IN_CHECK_RESULT_BREAK;
                        }
                        break;

                    case ABILITY_SAND_STREAM:
                        battleCtx->battleMons[battler].weatherAbilityAnnounced = TRUE;

                        if ((battleCtx->fieldConditionsMask & FIELD_CONDITION_SANDSTORM_PERM) == FALSE) {
                            subscript = BATTLE_SUBSEQ_SAND_STREAM;
                            result = SWITCH_IN_CHECK_RESULT_BREAK;
                        }
                        break;

                    case ABILITY_DROUGHT:
                        battleCtx->battleMons[battler].weatherAbilityAnnounced = TRUE;

                        if ((battleCtx->fieldConditionsMask & FIELD_CONDITION_SUNNY_PERM) == FALSE) {
                            subscript = BATTLE_SUBSEQ_DROUGHT;
                            result = SWITCH_IN_CHECK_RESULT_BREAK;
                        }
                        break;

                    case ABILITY_SNOW_WARNING:
                        battleCtx->battleMons[battler].weatherAbilityAnnounced = TRUE;

                        if ((battleCtx->fieldConditionsMask & FIELD_CONDITION_HAILING_PERM) == FALSE) {
                            subscript = BATTLE_SUBSEQ_SNOW_WARNING;
                            result = SWITCH_IN_CHECK_RESULT_BREAK;
                        }
                        break;
                    }
                }

                if (result == SWITCH_IN_CHECK_RESULT_BREAK) {
                    battleCtx->msgBattlerTemp = battler;
                    break;
                }
            }

            if (i == maxBattlers) {
                battleCtx->switchInCheckState++;
            }
            break;

        case SWITCH_IN_CHECK_STATE_INTIMIDATE:
            for (i = 0; i < maxBattlers; i++) {
                battler = battleCtx->monSpeedOrder[i];

                if (battleCtx->battleMons[battler].intimidateAnnounced == FALSE
                        && battleCtx->battleMons[battler].curHP
                        && Battler_Ability(battleCtx, battler) == ABILITY_INTIMIDATE) {
                    battleCtx->battleMons[battler].intimidateAnnounced = TRUE;
                    battleCtx->msgBattlerTemp = battler;
                    subscript = BATTLE_SUBSEQ_INTIMIDATE;
                    result = TRUE;
                    break;
                }
            }

            if (i == maxBattlers) {
                battleCtx->switchInCheckState++;
            }
            break;

        case SWITCH_IN_CHECK_STATE_DOWNLOAD:
            for (i = 0; i < maxBattlers; i++) {
                battler = battleCtx->monSpeedOrder[i];

                if (battleCtx->battleMons[battler].downloadAnnounced == FALSE
                        && battleCtx->battleMons[battler].curHP
                        && Battler_Ability(battleCtx, battler) == ABILITY_DOWNLOAD) {
                    int j; // must declare here to match
                    int sumDef = 0, sumSpDef = 0;

                    for (j = 0; j < maxBattlers; j++) {
                        if (Battler_Side(battleSys, battler) != Battler_Side(battleSys, j)
                                && (battleCtx->battleMons[j].statusVolatile & VOLATILE_CONDITION_SUBSTITUTE) == FALSE
                                && battleCtx->battleMons[j].curHP) {
                            sumDef += battleCtx->battleMons[j].defense
                                    * sStatStageBoosts[battleCtx->battleMons[j].statBoosts[BATTLE_STAT_DEFENSE]].numerator
                                    / sStatStageBoosts[battleCtx->battleMons[j].statBoosts[BATTLE_STAT_DEFENSE]].denominator;
                            sumSpDef += battleCtx->battleMons[j].spDefense
                                    * sStatStageBoosts[battleCtx->battleMons[j].statBoosts[BATTLE_STAT_SP_DEFENSE]].numerator
                                    / sStatStageBoosts[battleCtx->battleMons[j].statBoosts[BATTLE_STAT_SP_DEFENSE]].denominator;
                        }
                    }

                    battleCtx->battleMons[battler].downloadAnnounced = TRUE;

                    if (sumDef + sumSpDef != 0) {
                        if (sumDef >= sumSpDef) {
                            battleCtx->sideEffectParam = MOVE_SUBSCRIPT_PTR_SP_ATTACK_UP_1_STAGE;
                        } else {
                            battleCtx->sideEffectParam = MOVE_SUBSCRIPT_PTR_ATTACK_UP_1_STAGE;
                        }

                        battleCtx->sideEffectType = SIDE_EFFECT_TYPE_ABILITY;
                        battleCtx->sideEffectMon = battler;
                        subscript = BATTLE_SUBSEQ_UPDATE_STAT_STAGE;
                        result = SWITCH_IN_CHECK_RESULT_BREAK;
                        break;
                    }
                }
            }

            if (i == maxBattlers) {
                battleCtx->switchInCheckState++;
            }
            break;

        case SWITCH_IN_CHECK_STATE_ANTICIPATION:
            for (i = 0; i < maxBattlers; i++) {
                battler = battleCtx->monSpeedOrder[i];

                if (battleCtx->battleMons[battler].anticipationAnnounced == FALSE
                        && battleCtx->battleMons[battler].curHP
                        && Battler_Ability(battleCtx, battler) == ABILITY_ANTICIPATION) {
                    int j, k;
                    u16 move;
                    u32 effectiveness;

                    battleCtx->battleMons[battler].anticipationAnnounced = TRUE;

                    for (j = 0; j < maxBattlers; j++) {
                        if (Battler_Side(battleSys, battler) != Battler_Side(battleSys, j) && battleCtx->battleMons[j].curHP) {
                            for (k = 0; k < 4; k++) {
                                move = battleCtx->battleMons[j].moves[k];

                                if (move) {
                                    effectiveness = 0;
                                    battleCtx->damage = BattleSystem_CheckTypeChart(battleSys, battleCtx, move, NULL, j, battler, battleCtx->damage, &effectiveness);

                                    if ((effectiveness & MOVE_STATUS_INEFFECTIVE) == FALSE
                                            && MoveCannotTriggerAnticipation(battleCtx, move) == FALSE
                                            && ((effectiveness & MOVE_STATUS_SUPER_EFFECTIVE) 
                                                || (MOVE_DATA(move).effect == BATTLE_EFFECT_ONE_HIT_KO
                                                    && battleCtx->battleMons[battler].level <= battleCtx->battleMons[j].level))) {
                                        result = SWITCH_IN_CHECK_RESULT_BREAK;
                                        break;
                                    }
                                }
                            }

                            if (result == SWITCH_IN_CHECK_RESULT_BREAK) {
                                break;
                            }
                        }
                    }

                    if (result == SWITCH_IN_CHECK_RESULT_BREAK) {
                        battleCtx->msgBattlerTemp = battler;
                        subscript = BATTLE_SUBSEQ_ANTICIPATION;
                    }
                    break;
                }
            }

            if (i == maxBattlers) {
                battleCtx->switchInCheckState++;
            }
            break;

        case SWITCH_IN_CHECK_STATE_FOREWARN:
            for (i = 0; i < maxBattlers; i++) {
                battler = battleCtx->monSpeedOrder[i];

                if (battleCtx->battleMons[battler].forewarnAnnounced == FALSE
                        && battleCtx->battleMons[battler].curHP
                        && Battler_Ability(battleCtx, battler) == ABILITY_FOREWARN) {
                    int j, k;
                    u16 move;
                    u32 movePower, maxPower;
                    int sumEnemyHP;

                    battleCtx->battleMons[battler].forewarnAnnounced = TRUE;
                    maxPower = 0;
                    sumEnemyHP = 0;

                    for (j = 0; j < maxBattlers; j++) {
                        if (Battler_Side(battleSys, battler) != Battler_Side(battleSys, j)
                                && battleCtx->battleMons[j].curHP) {
                            sumEnemyHP += battleCtx->battleMons[j].curHP;

                            for (k = 0; k < 4; k++) {
                                move = battleCtx->battleMons[j].moves[k];
                                movePower = MOVE_DATA(move).power;

                                if (movePower == 1) {
                                    switch (MOVE_DATA(move).effect) {
                                    case BATTLE_EFFECT_ONE_HIT_KO:
                                        if (maxPower < 150 || (maxPower == 150 && (BattleSystem_RandNext(battleSys) & 1))) {
                                            maxPower = 150;
                                            battleCtx->msgMoveTemp = move;
                                        }
                                        break;

                                    case BATTLE_EFFECT_COUNTER:
                                    case BATTLE_EFFECT_MIRROR_COAT:
                                    case BATTLE_EFFECT_METAL_BURST:
                                        if (maxPower < 120 || (maxPower == 120 && (BattleSystem_RandNext(battleSys) & 1))) {
                                            maxPower = 120;
                                            battleCtx->msgMoveTemp = move;
                                        }
                                        break;

                                    default:
                                        if (maxPower < 80 || (maxPower == 80 && (BattleSystem_RandNext(battleSys) & 1))) {
                                            maxPower = 80;
                                            battleCtx->msgMoveTemp = move;
                                        }
                                        break;
                                    }
                                } else if (maxPower < movePower || (maxPower == movePower && (BattleSystem_RandNext(battleSys) & 1))) {
                                    maxPower = movePower;
                                    battleCtx->msgMoveTemp = move;
                                }
                            }
                        }
                    }

                    if (maxPower) {
                        battleCtx->msgBattlerTemp = battler;

                        subscript = BATTLE_SUBSEQ_FOREWARN;
                        result = SWITCH_IN_CHECK_RESULT_BREAK;
                    } else if (sumEnemyHP) {
                        j = BattleSystem_RandomOpponent(battleSys, battleCtx, battler);
                        k = Battler_CountMoves(battleSys, battleCtx, j);

                        battleCtx->msgMoveTemp = battleCtx->battleMons[j].moves[BattleSystem_RandNext(battleSys) % k];
                        battleCtx->msgBattlerTemp = battler;

                        subscript = BATTLE_SUBSEQ_FOREWARN;
                        result = SWITCH_IN_CHECK_RESULT_BREAK;
                    }
                    break;
                }
            }

            if (i == maxBattlers) {
                battleCtx->switchInCheckState++;
            }
            break;

        case SWITCH_IN_CHECK_STATE_FRISK:
            for (i = 0; i < maxBattlers; i++) {
                battler = battleCtx->monSpeedOrder[i];

                if (battleCtx->battleMons[battler].friskAnnounced == FALSE
                        && battleCtx->battleMons[battler].curHP
                        && Battler_Ability(battleCtx, battler) == ABILITY_FRISK) {
                    battleCtx->battleMons[battler].friskAnnounced = TRUE;

                    if (BattleSystem_BattleType(battleSys) & BATTLE_TYPE_DOUBLES) {
                        int enemies[] = {
                            BattleSystem_EnemyInSlot(battleSys, battler, ENEMY_IN_SLOT_RIGHT),
                            BattleSystem_EnemyInSlot(battleSys, battler, ENEMY_IN_SLOT_LEFT),
                        };

                        if (battleCtx->battleMons[enemies[0]].curHP
                                && battleCtx->battleMons[enemies[0]].heldItem
                                && battleCtx->battleMons[enemies[1]].curHP
                                && battleCtx->battleMons[enemies[1]].heldItem) {
                            battleCtx->msgItemTemp = battleCtx->battleMons[enemies[BattleSystem_RandNext(battleSys) & 1]].heldItem;
                            result = SWITCH_IN_CHECK_RESULT_BREAK;
                        } else if (battleCtx->battleMons[enemies[0]].curHP
                                && battleCtx->battleMons[enemies[0]].heldItem) {
                            battleCtx->msgItemTemp = battleCtx->battleMons[enemies[0]].heldItem;
                            result = SWITCH_IN_CHECK_RESULT_BREAK;
                        } else if (battleCtx->battleMons[enemies[1]].curHP
                                && battleCtx->battleMons[enemies[1]].heldItem) {
                            battleCtx->msgItemTemp = battleCtx->battleMons[enemies[1]].heldItem;
                            result = SWITCH_IN_CHECK_RESULT_BREAK;
                        }
                    } else if (battleCtx->battleMons[battler ^ 1].curHP && battleCtx->battleMons[battler ^ 1].heldItem) {
                        battleCtx->msgItemTemp = battleCtx->battleMons[battler ^ 1].heldItem;
                        result = SWITCH_IN_CHECK_RESULT_BREAK;
                    }
                }

                if (result == SWITCH_IN_CHECK_RESULT_BREAK) {
                    battleCtx->msgBattlerTemp = battler;
                    subscript = BATTLE_SUBSEQ_FRISK;
                    break;
                }
            }

            if (i == maxBattlers) {
                battleCtx->switchInCheckState++;
            }
            break;

        case SWITCH_IN_CHECK_STATE_SLOW_START:
            for (i = 0; i < maxBattlers; i++) {
                battler = battleCtx->monSpeedOrder[i];

                if (battleCtx->battleMons[battler].slowStartAnnounced == FALSE
                        && battleCtx->battleMons[battler].curHP
                        && Battler_Ability(battleCtx, battler) == ABILITY_SLOW_START
                        && battleCtx->totalTurns <= battleCtx->battleMons[battler].moveEffectsData.slowStartTurnNumber) {
                    battleCtx->battleMons[battler].slowStartAnnounced = TRUE;
                    battleCtx->msgBattlerTemp = battler;
                    subscript = BATTLE_SUBSEQ_SLOW_START;
                    result = SWITCH_IN_CHECK_RESULT_BREAK;
                    break;
                }

                if ((battleCtx->battleMons[battler].slowStartFinished == 0) && (battleCtx->battleMons[battler].curHP) && (Battler_Ability(battleCtx, battler) == 112) && ((battleCtx->totalTurns - battleCtx->battleMons[battler].moveEffectsData.slowStartTurnNumber) == 5)) {
                    battleCtx->battleMons[battler].slowStartFinished = 1;
                    battleCtx->msgBattlerTemp = battler;
                    subscript = BATTLE_SUBSEQ_SLOW_START_END;
                    result = SWITCH_IN_CHECK_RESULT_BREAK;
                    break;
                }
            }

            if (i == maxBattlers) {
                battleCtx->switchInCheckState++;
            }
            break;
    
        case SWITCH_IN_CHECK_STATE_MOLD_BREAKER:
            for (i = 0; i < maxBattlers; i++) {
                battler = battleCtx->monSpeedOrder[i];

                if (battleCtx->battleMons[battler].moldBreakerAnnounced == FALSE
                        && battleCtx->battleMons[battler].curHP
                        && Battler_Ability(battleCtx, battler) == ABILITY_MOLD_BREAKER) {
                    battleCtx->battleMons[battler].moldBreakerAnnounced = TRUE;
                    battleCtx->msgBattlerTemp = battler;
                    subscript = BATTLE_SUBSEQ_MOLD_BREAKER;
                    result = SWITCH_IN_CHECK_RESULT_BREAK;
                    break;
                }
            }

            if (i == maxBattlers) {
                battleCtx->switchInCheckState++;
            }
            break;

        case SWITCH_IN_CHECK_STATE_PRESSURE:
            for (i = 0; i < maxBattlers; i++) {
                battler = battleCtx->monSpeedOrder[i];

                if (battleCtx->battleMons[battler].pressureAnnounced == FALSE
                        && battleCtx->battleMons[battler].curHP
                        && Battler_Ability(battleCtx, battler) == ABILITY_PRESSURE) {
                    battleCtx->battleMons[battler].pressureAnnounced = TRUE;
                    battleCtx->msgBattlerTemp = battler;
                    subscript = BATTLE_SUBSEQ_PRESSURE;
                    result = SWITCH_IN_CHECK_RESULT_BREAK;
                    break;
                }
            }

            if (i == maxBattlers) {
                battleCtx->switchInCheckState++;
            }
            break;

        case SWITCH_IN_CHECK_STATE_FORM_CHANGE:
            if (BattleSystem_TriggerFormChange(battleSys, battleCtx, &subscript) == TRUE) {
                result = SWITCH_IN_CHECK_RESULT_BREAK;
            } else {
                battleCtx->switchInCheckState++;
            }
            break;

        case SWITCH_IN_CHECK_STATE_AMULET_COIN:
            for (i = 0; i < maxBattlers; i++) {
                battler = battleCtx->monSpeedOrder[i];

                if (BattleSystem_GetItemData(battleCtx, battleCtx->battleMons[battler].heldItem, ITEM_PARAM_HOLD_EFFECT) == HOLD_EFFECT_MONEY_UP) {
                    battleCtx->prizeMoneyMul = 2;
                }
            }

            battleCtx->switchInCheckState++;
            break;

        case SWITCH_IN_CHECK_STATE_FORBIDDEN_STATUS:
            for (i = 0; i < maxBattlers; i++) {
                battler = battleCtx->monSpeedOrder[i];

                if (BattleSystem_RecoverStatusByAbility(battleSys, battleCtx, battler, TRUE) == TRUE) {
                    subscript = BATTLE_SUBSEQ_ABILITY_FORBIDS_STATUS;
                    result = SWITCH_IN_CHECK_RESULT_BREAK;
                    break;
                }
            }

            if (i == maxBattlers) {
                battleCtx->switchInCheckState++;
            }
            break;

        case SWITCH_IN_CHECK_STATE_HELD_ITEM_STATUS:
            for (i = 0; i < maxBattlers; i++) {
                battler = battleCtx->monSpeedOrder[i];

                if (BattleSystem_TriggerHeldItemOnStatus(battleSys, battleCtx, battler, &subscript) == TRUE) { // document
                    battleCtx->msgBattlerTemp = battler;
                    result = SWITCH_IN_CHECK_RESULT_BREAK;
                    break;
                }
            }

            if (i == maxBattlers) {
                battleCtx->switchInCheckState++;
            }
            break;

        case SWITCH_IN_CHECK_STATE_DONE:
            battleCtx->switchInCheckState = 0;
            result = SWITCH_IN_CHECK_RESULT_DONE;
            break;
        }
    } while (result == SWITCH_IN_CHECK_RESULT_CONTINUE);

    return subscript;
}

int BattleSystem_RandomOpponent(BattleSystem *battleSys, BattleContext *battleCtx, int attacker)
{
    int opponents[2];
    u32 battleType = BattleSystem_BattleType(battleSys);

    int chosen;
    if (battleType & BATTLE_TYPE_DOUBLES) {
        opponents[0] = BattleSystem_EnemyInSlot(battleSys, attacker, ENEMY_IN_SLOT_RIGHT);
        opponents[1] = BattleSystem_EnemyInSlot(battleSys, attacker, ENEMY_IN_SLOT_LEFT);
        
        int rnd = BattleSystem_RandNext(battleSys) & 1;
        chosen = opponents[rnd];

        if (battleCtx->battleMons[chosen].curHP == 0) {
            chosen = opponents[rnd ^ 1];
        }
    } else {
        chosen = attacker ^ 1;
    }

    return chosen;
}

BOOL BattleSystem_TriggerAbilityOnHit(BattleSystem *battleSys, BattleContext *battleCtx, int *subscript)
{
    BOOL result = FALSE;

    // These two sentinels must be separate to match
    if (battleCtx->defender == BATTLER_NONE) {
        return result;
    }

    if (Battler_SubstituteWasHit(battleCtx, battleCtx->defender) == TRUE) {
        return result;
    }

    switch (Battler_Ability(battleCtx, battleCtx->defender)) {
    case ABILITY_STATIC:
        if (ATTACKING_MON.curHP
                && ATTACKING_MON.status == MON_CONDITION_NONE
                && (battleCtx->moveStatusFlags & MOVE_STATUS_NO_EFFECTS) == FALSE
                && (battleCtx->battleStatusMask & SYSCTL_FIRST_OF_MULTI_TURN) == FALSE
                && (battleCtx->battleStatusMask2 & SYSCTL_UTURN_ACTIVE) == FALSE
                && (DEFENDER_SELF_TURN_FLAGS.physicalDamageTaken || DEFENDER_SELF_TURN_FLAGS.specialDamageTaken)
                && (CURRENT_MOVE_DATA.flags & MOVE_FLAG_MAKES_CONTACT)
                && BattleSystem_RandNext(battleSys) % 10 < 3) {
            battleCtx->sideEffectType = SIDE_EFFECT_TYPE_ABILITY;
            battleCtx->sideEffectMon = battleCtx->attacker;
            battleCtx->msgBattlerTemp = battleCtx->defender;

            *subscript = BATTLE_SUBSEQ_PARALYZE;
            result = TRUE;
        }
        break;

    case ABILITY_COLOR_CHANGE:
        u8 moveType;

        if (Battler_Ability(battleCtx, battleCtx->attacker) == ABILITY_NORMALIZE) {
            moveType = TYPE_NORMAL;
        } else if (battleCtx->moveType) {
            moveType = battleCtx->moveType;
        } else {
            moveType = CURRENT_MOVE_DATA.type;
        }

        if (DEFENDING_MON.curHP
                && (battleCtx->moveStatusFlags & MOVE_STATUS_NO_EFFECTS) == FALSE
                && battleCtx->moveCur != MOVE_STRUGGLE
                && (DEFENDER_SELF_TURN_FLAGS.physicalDamageTaken || DEFENDER_SELF_TURN_FLAGS.specialDamageTaken)
                && (battleCtx->battleStatusMask2 & SYSCTL_UTURN_ACTIVE) == FALSE
                && CURRENT_MOVE_DATA.power
                && BattleMon_Get(battleCtx, battleCtx->defender, 27, NULL) != moveType
                && BattleMon_Get(battleCtx, battleCtx->defender, 28, NULL) != moveType) {
            *subscript = BATTLE_SUBSEQ_COLOR_CHANGE;
            battleCtx->msgTemp = moveType;
            result = TRUE;
        }
        break;

    case ABILITY_ROUGH_SKIN:
        if (ATTACKING_MON.curHP
                && Battler_Ability(battleCtx, battleCtx->attacker) != ABILITY_MAGIC_GUARD
                && (battleCtx->moveStatusFlags & MOVE_STATUS_NO_EFFECTS) == FALSE
                && (battleCtx->battleStatusMask & SYSCTL_FIRST_OF_MULTI_TURN) == FALSE
                && (battleCtx->battleStatusMask2 & SYSCTL_UTURN_ACTIVE) == FALSE
                && (DEFENDER_SELF_TURN_FLAGS.physicalDamageTaken || DEFENDER_SELF_TURN_FLAGS.specialDamageTaken)
                && (CURRENT_MOVE_DATA.flags & MOVE_FLAG_MAKES_CONTACT)) {
            battleCtx->hpCalcTemp = BattleSystem_Divide(ATTACKING_MON.maxHP * -1, 8);
            battleCtx->msgBattlerTemp = battleCtx->attacker;

            *subscript = BATTLE_SUBSEQ_ROUGH_SKIN;
            result = TRUE;
        }
        break;

    case ABILITY_EFFECT_SPORE:
        if (ATTACKING_MON.curHP
                && ATTACKING_MON.status == MON_CONDITION_NONE
                && (battleCtx->moveStatusFlags & MOVE_STATUS_NO_EFFECTS) == FALSE
                && (battleCtx->battleStatusMask & SYSCTL_FIRST_OF_MULTI_TURN) == FALSE
                && (battleCtx->battleStatusMask2 & SYSCTL_UTURN_ACTIVE) == FALSE
                && (DEFENDER_SELF_TURN_FLAGS.physicalDamageTaken || DEFENDER_SELF_TURN_FLAGS.specialDamageTaken)
                && (CURRENT_MOVE_DATA.flags & MOVE_FLAG_MAKES_CONTACT)
                && BattleSystem_RandNext(battleSys) % 10 < 3) {
            switch (BattleSystem_RandNext(battleSys) % 3) {
            case 0:
            default:
                *subscript = BATTLE_SUBSEQ_POISON;
                break;
            case 1:
                *subscript = BATTLE_SUBSEQ_PARALYZE;
                break;
            case 2:
                *subscript = BATTLE_SUBSEQ_FALL_ASLEEP;
                break;
            }

            battleCtx->sideEffectType = SIDE_EFFECT_TYPE_ABILITY;
            battleCtx->sideEffectMon = battleCtx->attacker;
            battleCtx->msgBattlerTemp = battleCtx->defender;

            result = TRUE;
        }
        break;

    case ABILITY_POISON_POINT:
        if (ATTACKING_MON.curHP
                && ATTACKING_MON.status == MON_CONDITION_NONE
                && (battleCtx->moveStatusFlags & MOVE_STATUS_NO_EFFECTS) == FALSE
                && (battleCtx->battleStatusMask & SYSCTL_FIRST_OF_MULTI_TURN) == FALSE
                && (battleCtx->battleStatusMask2 & SYSCTL_UTURN_ACTIVE) == FALSE
                && (DEFENDER_SELF_TURN_FLAGS.physicalDamageTaken || DEFENDER_SELF_TURN_FLAGS.specialDamageTaken)
                && (CURRENT_MOVE_DATA.flags & MOVE_FLAG_MAKES_CONTACT)
                && BattleSystem_RandNext(battleSys) % 10 < 3) {
            battleCtx->sideEffectType = SIDE_EFFECT_TYPE_ABILITY;
            battleCtx->sideEffectMon = battleCtx->attacker;
            battleCtx->msgBattlerTemp = battleCtx->defender;

            *subscript = BATTLE_SUBSEQ_POISON;
            result = TRUE;
        }
        break;

    case ABILITY_FLAME_BODY:
        if (ATTACKING_MON.curHP
                && ATTACKING_MON.status == MON_CONDITION_NONE
                && (battleCtx->moveStatusFlags & MOVE_STATUS_NO_EFFECTS) == FALSE
                && (battleCtx->battleStatusMask & SYSCTL_FIRST_OF_MULTI_TURN) == FALSE
                && (battleCtx->battleStatusMask2 & SYSCTL_UTURN_ACTIVE) == FALSE
                && (DEFENDER_SELF_TURN_FLAGS.physicalDamageTaken || DEFENDER_SELF_TURN_FLAGS.specialDamageTaken)
                && (CURRENT_MOVE_DATA.flags & MOVE_FLAG_MAKES_CONTACT)
                && BattleSystem_RandNext(battleSys) % 10 < 3) {
            battleCtx->sideEffectType = SIDE_EFFECT_TYPE_ABILITY;
            battleCtx->sideEffectMon = battleCtx->attacker;
            battleCtx->msgBattlerTemp = battleCtx->defender;

            *subscript = BATTLE_SUBSEQ_BURN;
            result = TRUE;
        }
        break;

    case ABILITY_CUTE_CHARM:
        if (ATTACKING_MON.curHP
                && (ATTACKING_MON.statusVolatile & VOLATILE_CONDITION_ATTRACT) == FALSE
                && (battleCtx->moveStatusFlags & MOVE_STATUS_NO_EFFECTS) == FALSE
                && (battleCtx->battleStatusMask & SYSCTL_FIRST_OF_MULTI_TURN) == FALSE
                && (battleCtx->battleStatusMask2 & SYSCTL_UTURN_ACTIVE) == FALSE
                && (DEFENDER_SELF_TURN_FLAGS.physicalDamageTaken || DEFENDER_SELF_TURN_FLAGS.specialDamageTaken)
                && (CURRENT_MOVE_DATA.flags & MOVE_FLAG_MAKES_CONTACT)
                && DEFENDING_MON.curHP
                && BattleSystem_RandNext(battleSys) % 10 < 3) {
            battleCtx->sideEffectType = SIDE_EFFECT_TYPE_ABILITY;
            battleCtx->sideEffectMon = battleCtx->attacker;
            battleCtx->msgBattlerTemp = battleCtx->defender;

            *subscript = BATTLE_SUBSEQ_INFATUATE;
            result = TRUE;
        }
        break;

    case ABILITY_AFTERMATH:
        if (battleCtx->defender == battleCtx->faintedMon
                && Battler_Ability(battleCtx, battleCtx->attacker) != ABILITY_MAGIC_GUARD
                && BattleSystem_CountAbility(battleSys, battleCtx, 8, 0, ABILITY_DAMP) == 0
                && (battleCtx->battleStatusMask2 & SYSCTL_UTURN_ACTIVE) == FALSE
                && ATTACKING_MON.curHP
                && (battleCtx->moveStatusFlags & MOVE_STATUS_NO_EFFECTS) == FALSE
                && (CURRENT_MOVE_DATA.flags & MOVE_FLAG_MAKES_CONTACT)) {
            battleCtx->hpCalcTemp = BattleSystem_Divide(ATTACKING_MON.maxHP * -1, 4);
            battleCtx->msgBattlerTemp = battleCtx->attacker;

            *subscript = BATTLE_SUBSEQ_AFTERMATH;
            result = TRUE;
        }
        break;
    }

    return result;
}

BOOL BattleSystem_RecoverStatusByAbility(BattleSystem *battleSys, BattleContext *battleCtx, int battler, int skipLoad)
{
    BOOL result = FALSE;

    switch (Battler_Ability(battleCtx, battler)) {
    case ABILITY_IMMUNITY:
        if (battleCtx->battleMons[battler].status & MON_CONDITION_ANY_POISON) {
            battleCtx->msgTemp = MSGCOND_POISON;
            result = TRUE;
        }
        break;

    case ABILITY_OWN_TEMPO:
        if (battleCtx->battleMons[battler].statusVolatile & VOLATILE_CONDITION_CONFUSION) {
            battleCtx->msgTemp = MSGCOND_CONFUSION;
            result = TRUE;
        }
        break;

    case ABILITY_LIMBER:
        if (battleCtx->battleMons[battler].status & MON_CONDITION_PARALYSIS) {
            battleCtx->msgTemp = MSGCOND_PARALYSIS;
            result = TRUE;
        }
        break;

    case ABILITY_INSOMNIA:
    case ABILITY_VITAL_SPIRIT:
        if (battleCtx->battleMons[battler].status & MON_CONDITION_SLEEP) {
            battleCtx->msgTemp = MSGCOND_SLEEP;
            result = TRUE;
        }
        break;

    case ABILITY_WATER_VEIL:
        if (battleCtx->battleMons[battler].status & MON_CONDITION_BURN) {
            battleCtx->msgTemp = MSGCOND_BURN;
            result = TRUE;
        }
        break;

    case ABILITY_MAGMA_ARMOR:
        if (battleCtx->battleMons[battler].status & MON_CONDITION_FREEZE) {
            battleCtx->msgTemp = MSGCOND_FREEZE;
            result = TRUE;
        }
        break;

    case ABILITY_OBLIVIOUS:
        if (battleCtx->battleMons[battler].statusVolatile & VOLATILE_CONDITION_ATTRACT) {
            battleCtx->msgTemp = MSGCOND_INFATUATION;
            result = TRUE;
        }
        break;

    case ABILITY_UNBURDEN:
        if (battleCtx->battleMons[battler].heldItem) {
            battleCtx->battleMons[battler].moveEffectsData.canUnburden = TRUE;
        }
        break;
    }

    if (result == TRUE) {
        battleCtx->msgBattlerTemp = battler;
        battleCtx->msgAbilityTemp = Battler_Ability(battleCtx, battler);

        if (skipLoad == FALSE) {
            LOAD_SUBSEQ(BATTLE_SUBSEQ_ABILITY_FORBIDS_STATUS);
            battleCtx->commandNext = battleCtx->command;
            battleCtx->command = BATTLE_CONTROL_EXEC_SCRIPT;
        }
    }

    return result;
}

BOOL Ability_ForbidsStatus(BattleContext *battleSys, int ability, int status)
{
    BOOL result = FALSE;

    switch (ability) {
    case ABILITY_IMMUNITY:
        if (status & MON_CONDITION_ANY_POISON) {
            result = TRUE;
        }
        break;

    case ABILITY_LIMBER:
        if (status & MON_CONDITION_PARALYSIS) {
            result = TRUE;
        }
        break;

    case ABILITY_INSOMNIA:
    case ABILITY_VITAL_SPIRIT:
        if (status & MON_CONDITION_SLEEP) {
            result = TRUE;
        }
        break;

    case ABILITY_WATER_VEIL:
        if (status & MON_CONDITION_BURN) {
            result = TRUE;
        }
        break;
        
    case ABILITY_MAGMA_ARMOR:
        if (status & MON_CONDITION_FREEZE) {
            result = TRUE;
        }
        break;
    }

    return result;
}

BOOL BattleSystem_SynchronizeStatus(BattleSystem * battleSys, BattleContext * battleCtx, int controllerCommand)
{
    BOOL result = FALSE;
    int nextSeq = 0;

    if (battleCtx->defender != BATTLER_NONE
            && Battler_Ability(battleCtx, battleCtx->defender) == ABILITY_SYNCHRONIZE
            && battleCtx->defender == battleCtx->sideEffectMon
            && (battleCtx->battleStatusMask & SYSCTL_TRY_SYNCHRONIZE_STATUS)) {
        battleCtx->msgBattlerTemp = battleCtx->defender;
        battleCtx->sideEffectMon = battleCtx->attacker;
        result = TRUE;
    } else if (Battler_Ability(battleCtx, battleCtx->attacker) == ABILITY_SYNCHRONIZE
            && battleCtx->attacker == battleCtx->sideEffectMon
            && (battleCtx->battleStatusMask & SYSCTL_TRY_SYNCHRONIZE_STATUS)) {
        // This will try to synchronize a status applied from an ability or item held
        // by the defender, e.g. Static.
        battleCtx->msgBattlerTemp = battleCtx->attacker;
        battleCtx->sideEffectMon = battleCtx->defender;
        result = TRUE;
    }

    if (result == TRUE) {
        if (battleCtx->battleMons[battleCtx->msgBattlerTemp].status & MON_CONDITION_ANY_POISON) {
            nextSeq = BATTLE_SUBSEQ_POISON;
        } else if (battleCtx->battleMons[battleCtx->msgBattlerTemp].status & MON_CONDITION_BURN) {
            nextSeq = BATTLE_SUBSEQ_BURN;
        } else if (battleCtx->battleMons[battleCtx->msgBattlerTemp].status & MON_CONDITION_PARALYSIS) {
            nextSeq = BATTLE_SUBSEQ_PARALYZE;
        }

        if (nextSeq) {
            battleCtx->sideEffectType = SIDE_EFFECT_TYPE_ABILITY;

            LOAD_SUBSEQ(nextSeq);
            battleCtx->commandNext = controllerCommand;
            battleCtx->command = BATTLE_CONTROL_EXEC_SCRIPT;

            return result;
        }
    }

    result = BattleSystem_TriggerFormChange(battleSys, battleCtx, &nextSeq);
    if (result == TRUE) {
        LOAD_SUBSEQ(nextSeq);
        battleCtx->commandNext = controllerCommand;
        battleCtx->command = BATTLE_CONTROL_EXEC_SCRIPT;

        return result;
    }

    if (battleCtx->defender != BATTLER_NONE
            && Battler_HeldItemEffect(battleCtx, battleCtx->defender) == HOLD_EFFECT_RECIPROCATE_INFAT
            && battleCtx->defender == battleCtx->sideEffectMon
            && (DEFENDER_SELF_TURN_FLAGS.statusFlags & SELF_TURN_FLAG_INFATUATED)) {
        battleCtx->msgBattlerTemp = battleCtx->defender;
        battleCtx->sideEffectMon = battleCtx->attacker;
        result = TRUE;
    } else if (Battler_HeldItemEffect(battleCtx, battleCtx->attacker) == HOLD_EFFECT_RECIPROCATE_INFAT
            && battleCtx->attacker == battleCtx->sideEffectMon
            && (ATTACKER_SELF_TURN_FLAGS.statusFlags & SELF_TURN_FLAG_INFATUATED)) {
        // This should only happen if the defender triggered Cute Charm.
        battleCtx->msgBattlerTemp = battleCtx->attacker;
        battleCtx->sideEffectMon = battleCtx->defender;
        result = TRUE;
    }

    if (result == TRUE) {
        nextSeq = BATTLE_SUBSEQ_INFATUATE;
        battleCtx->sideEffectType = SIDE_EFFECT_TYPE_HELD_ITEM;

        LOAD_SUBSEQ(nextSeq);
        battleCtx->commandNext = controllerCommand;
        battleCtx->command = BATTLE_CONTROL_EXEC_SCRIPT;

        return result; // This line has to stay to match.
    }

    return FALSE;
}

BOOL BattleSystem_TriggerHeldItem(BattleSystem *battleSys, BattleContext *battleCtx, int battler)
{
    BOOL result = FALSE;
    int subscript;
    int itemEffect = Battler_HeldItemEffect(battleCtx, battler);
    int itemPower = Battler_HeldItemPower(battleCtx, battler, ITEM_POWER_CHECK_ALL);

    if (battleCtx->battleMons[battler].curHP) {
        switch (itemEffect) {
        case HOLD_EFFECT_HP_RESTORE:
            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / 2) {
                battleCtx->hpCalcTemp = itemPower;
                subscript = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_HP_PCT_RESTORE:
            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / 2) {
                battleCtx->hpCalcTemp = BattleSystem_Divide(battleCtx->battleMons[battler].maxHP * itemPower, 100);
                subscript = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_PRZ_RESTORE:
            if (battleCtx->battleMons[battler].status & MON_CONDITION_PARALYSIS) {
                subscript = BATTLE_SUBSEQ_HELD_ITEM_PRZ_RESTORE;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_SLP_RESTORE:
            if (battleCtx->battleMons[battler].status & MON_CONDITION_SLEEP) {
                subscript = BATTLE_SUBSEQ_HELD_ITEM_SLP_RESTORE;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_PSN_RESTORE:
            if (battleCtx->battleMons[battler].status & MON_CONDITION_ANY_POISON) {
                subscript = BATTLE_SUBSEQ_HELD_ITEM_PSN_RESTORE;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_BRN_RESTORE:
            if (battleCtx->battleMons[battler].status & MON_CONDITION_BURN) {
                subscript = BATTLE_SUBSEQ_HELD_ITEM_BRN_RESTORE;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_FRZ_RESTORE:
            if (battleCtx->battleMons[battler].status & MON_CONDITION_FREEZE) {
                subscript = BATTLE_SUBSEQ_HELD_ITEM_FRZ_RESTORE;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_PP_RESTORE: {
            int i;
            for (i = 0; i < LEARNED_MOVES_MAX; i++) {
                if (battleCtx->battleMons[battler].moves[i] && battleCtx->battleMons[battler].ppCur[i] == 0) {
                    break;
                }
            }

            if (i != LEARNED_MOVES_MAX) {
                BattleMon_AddVal(&battleCtx->battleMons[battler], BATTLEMON_CUR_PP_1 + i, itemPower);
                BattleMon_CopyToParty(battleSys, battleCtx, battler);
                battleCtx->msgMoveTemp = battleCtx->battleMons[battler].moves[i];
                subscript = BATTLE_SUBSEQ_HELD_ITEM_PP_RESTORE;
                result = TRUE;
            }

            break;
        }

        case HOLD_EFFECT_CONFUSE_RESTORE:
            if (battleCtx->battleMons[battler].statusVolatile & VOLATILE_CONDITION_CONFUSION) {
                subscript = BATTLE_SUBSEQ_HELD_ITEM_CNF_RESTORE;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_STATUS_RESTORE:
            if ((battleCtx->battleMons[battler].status & MON_CONDITION_ANY)
                    || (battleCtx->battleMons[battler].statusVolatile & VOLATILE_CONDITION_CONFUSION)) {
                if (battleCtx->battleMons[battler].status & MON_CONDITION_PARALYSIS) {
                    subscript = BATTLE_SUBSEQ_HELD_ITEM_PRZ_RESTORE;
                }

                if (battleCtx->battleMons[battler].status & MON_CONDITION_SLEEP) {
                    subscript = BATTLE_SUBSEQ_HELD_ITEM_SLP_RESTORE;
                }

                if (battleCtx->battleMons[battler].status & MON_CONDITION_ANY_POISON) {
                    subscript = BATTLE_SUBSEQ_HELD_ITEM_PSN_RESTORE;
                }

                if (battleCtx->battleMons[battler].status & MON_CONDITION_BURN) {
                    subscript = BATTLE_SUBSEQ_HELD_ITEM_BRN_RESTORE;
                }

                if (battleCtx->battleMons[battler].status & MON_CONDITION_FREEZE) {
                    subscript = BATTLE_SUBSEQ_HELD_ITEM_FRZ_RESTORE;
                }

                if (battleCtx->battleMons[battler].statusVolatile & VOLATILE_CONDITION_CONFUSION) {
                    subscript = BATTLE_SUBSEQ_HELD_ITEM_CNF_RESTORE;
                }

                if ((battleCtx->battleMons[battler].status & MON_CONDITION_ANY)
                        && (battleCtx->battleMons[battler].statusVolatile & VOLATILE_CONDITION_CONFUSION)) {
                    subscript = BATTLE_SUBSEQ_HELD_ITEM_MULTI_RESTORE;
                }

                result = TRUE;
            }
            break;

        case HOLD_EFFECT_HP_RESTORE_SPICY:
            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / 2) {
                battleCtx->hpCalcTemp = BattleSystem_Divide(battleCtx->battleMons[battler].maxHP, itemPower);
                battleCtx->msgTemp = FLAVOR_SPICY;

                if (Pokemon_GetFlavorAffinityOf(battleCtx->battleMons[battler].personality, FLAVOR_SPICY) == -1) {
                    subscript = BATTLE_SUBSEQ_HELD_ITEM_DISLIKE_FLAVOR;
                } else {
                    subscript = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
                }

                result = TRUE;
            }
            break;

        case HOLD_EFFECT_HP_RESTORE_DRY:
            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / 2) {
                battleCtx->hpCalcTemp = BattleSystem_Divide(battleCtx->battleMons[battler].maxHP, itemPower);
                battleCtx->msgTemp = FLAVOR_DRY;

                if (Pokemon_GetFlavorAffinityOf(battleCtx->battleMons[battler].personality, FLAVOR_DRY) == -1) {
                    subscript = BATTLE_SUBSEQ_HELD_ITEM_DISLIKE_FLAVOR;
                } else {
                    subscript = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
                }

                result = TRUE;
            }
            break;

        case HOLD_EFFECT_HP_RESTORE_SWEET:
            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / 2) {
                battleCtx->hpCalcTemp = BattleSystem_Divide(battleCtx->battleMons[battler].maxHP, itemPower);
                battleCtx->msgTemp = FLAVOR_SWEET;

                if (Pokemon_GetFlavorAffinityOf(battleCtx->battleMons[battler].personality, FLAVOR_SWEET) == -1) {
                    subscript = BATTLE_SUBSEQ_HELD_ITEM_DISLIKE_FLAVOR;
                } else {
                    subscript = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
                }

                result = TRUE;
            }
            break;

        case HOLD_EFFECT_HP_RESTORE_BITTER:
            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / 2) {
                battleCtx->hpCalcTemp = BattleSystem_Divide(battleCtx->battleMons[battler].maxHP, itemPower);
                battleCtx->msgTemp = FLAVOR_BITTER;

                if (Pokemon_GetFlavorAffinityOf(battleCtx->battleMons[battler].personality, FLAVOR_BITTER) == -1) {
                    subscript = BATTLE_SUBSEQ_HELD_ITEM_DISLIKE_FLAVOR;
                } else {
                    subscript = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
                }

                result = TRUE;
            }
            break;

        case HOLD_EFFECT_HP_RESTORE_SOUR:
            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / 2) {
                battleCtx->hpCalcTemp = BattleSystem_Divide(battleCtx->battleMons[battler].maxHP, itemPower);
                battleCtx->msgTemp = FLAVOR_SOUR;

                if (Pokemon_GetFlavorAffinityOf(battleCtx->battleMons[battler].personality, FLAVOR_SOUR) == -1) {
                    subscript = BATTLE_SUBSEQ_HELD_ITEM_DISLIKE_FLAVOR;
                } else {
                    subscript = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
                }

                result = TRUE;
            }
            break;case HOLD_EFFECT_PINCH_ATK_UP:
            if (Battler_Ability(battleCtx, battler) == ABILITY_GLUTTONY) {
                itemPower /= 2;
            }

            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / itemPower
                    && battleCtx->battleMons[battler].statBoosts[BATTLE_STAT_ATTACK] < 12) {
                battleCtx->msgTemp = BATTLE_STAT_ATTACK;
                subscript = BATTLE_SUBSEQ_HELD_ITEM_RAISE_STAT;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_PINCH_DEF_UP:
            if (Battler_Ability(battleCtx, battler) == ABILITY_GLUTTONY) {
                itemPower /= 2;
            }

            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / itemPower
                    && battleCtx->battleMons[battler].statBoosts[BATTLE_STAT_DEFENSE] < 12) {
                battleCtx->msgTemp = BATTLE_STAT_DEFENSE;
                subscript = BATTLE_SUBSEQ_HELD_ITEM_RAISE_STAT;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_PINCH_SPEED_UP:
            if (Battler_Ability(battleCtx, battler) == ABILITY_GLUTTONY) {
                itemPower /= 2;
            }

            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / itemPower
                    && battleCtx->battleMons[battler].statBoosts[BATTLE_STAT_SPEED] < 12) {
                battleCtx->msgTemp = BATTLE_STAT_SPEED;
                subscript = BATTLE_SUBSEQ_HELD_ITEM_RAISE_STAT;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_PINCH_SPATK_UP:
            if (Battler_Ability(battleCtx, battler) == ABILITY_GLUTTONY) {
                itemPower /= 2;
            }

            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / itemPower
                    && battleCtx->battleMons[battler].statBoosts[BATTLE_STAT_SP_ATTACK] < 12) {
                battleCtx->msgTemp = BATTLE_STAT_SP_ATTACK;
                subscript = BATTLE_SUBSEQ_HELD_ITEM_RAISE_STAT;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_PINCH_SPDEF_UP:
            if (Battler_Ability(battleCtx, battler) == ABILITY_GLUTTONY) {
                itemPower /= 2;
            }

            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / itemPower
                    && battleCtx->battleMons[battler].statBoosts[BATTLE_STAT_SP_DEFENSE] < 12) {
                battleCtx->msgTemp = BATTLE_STAT_SP_DEFENSE;
                subscript = BATTLE_SUBSEQ_HELD_ITEM_RAISE_STAT;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_PINCH_CRITRATE_UP:
            if (Battler_Ability(battleCtx, battler) == ABILITY_GLUTTONY) {
                itemPower /= 2;
            }

            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / itemPower
                    && (battleCtx->battleMons[battler].statusVolatile & VOLATILE_CONDITION_FOCUS_ENERGY) == FALSE) {
                subscript = BATTLE_SUBSEQ_HELD_ITEM_RAISE_CRIT;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_PINCH_RANDOM_UP:
            if (Battler_Ability(battleCtx, battler) == ABILITY_GLUTTONY) {
                itemPower /= 2;
            }

            if (battleCtx->battleMons[battler].curHP <= (battleCtx->battleMons[battler].maxHP / itemPower)) {
                int i;
                for (i = 0; i < 5; i++) {
                    if (battleCtx->battleMons[battler].statBoosts[BATTLE_STAT_ATTACK + i] < 12) {
                        break;
                    }
                }

                if (i != 5) {
                    do {
                        i = BattleSystem_RandNext(battleSys) % 5;
                    } while (battleCtx->battleMons[battler].statBoosts[BATTLE_STAT_ATTACK + i] == 12);

                    battleCtx->msgTemp = BATTLE_STAT_ATTACK + i;
                    subscript = BATTLE_SUBSEQ_HELD_ITEM_SHARPLY_RAISE_STAT;
                    result = TRUE;
                }
            }
            break;

        case HOLD_EFFECT_STATDOWN_RESTORE: {
            for (int i = BATTLE_STAT_HP; i < BATTLE_STAT_MAX; i++) {
                if (battleCtx->battleMons[battler].statBoosts[i] < 6) {
                    battleCtx->battleMons[battler].statBoosts[i] = 6;
                    result = TRUE;
                }
            }

            if (result == TRUE) {
                subscript = BATTLE_SUBSEQ_HELD_ITEM_STATDOWN_RESTORE;
            }
            break;
        }
            
        case HOLD_EFFECT_HEAL_INFATUATION:
            if (battleCtx->battleMons[battler].statusVolatile & VOLATILE_CONDITION_ATTRACT) {
                battleCtx->msgTemp = MSGCOND_INFATUATION;
                subscript = BATTLE_SUBSEQ_HELD_ITEM_HEAL_INFATUATION;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_PINCH_ACC_UP:
            if (Battler_Ability(battleCtx, battler) == ABILITY_GLUTTONY) {
                itemPower /= 2;
            }

            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / itemPower) {
                subscript = BATTLE_SUBSEQ_HELD_ITEM_TEMP_ACC_UP;
                result = TRUE;
            }
            break;
        }

        if (result == TRUE) {
            battleCtx->msgBattlerTemp = battler;
            battleCtx->msgItemTemp = Battler_HeldItem(battleCtx, battler);

            LOAD_SUBSEQ(subscript);
            battleCtx->commandNext = battleCtx->command;
            battleCtx->command = BATTLE_CONTROL_EXEC_SCRIPT;
        }
    }

    return result;
}

BOOL BattleSystem_TriggerLeftovers(BattleSystem *battleSys, BattleContext *battleCtx, int battler)
{
    BOOL result = 0;
    int subscript;
    int itemEffect = Battler_HeldItemEffect(battleCtx, battler);
    int itemPower = Battler_HeldItemPower(battleCtx, battler, ITEM_POWER_CHECK_ALL);

    if (battleCtx->battleMons[battler].curHP) {
        switch (itemEffect) {
        case HOLD_EFFECT_HP_RESTORE_GRADUAL:
            if (battleCtx->battleMons[battler].curHP < (battleCtx->battleMons[battler].maxHP)) {
                battleCtx->hpCalcTemp = BattleSystem_Divide(battleCtx->battleMons[battler].maxHP, 16);
                subscript = BATTLE_SUBSEQ_RESTORE_A_LITTLE_HP;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_HP_RESTORE_PSN_TYPE:
            if (MON_HAS_TYPE(battler, TYPE_POISON)) {
                if (battleCtx->battleMons[battler].curHP < (battleCtx->battleMons[battler].maxHP)) {
                    battleCtx->hpCalcTemp = BattleSystem_Divide(battleCtx->battleMons[battler].maxHP, 16);
                    subscript = BATTLE_SUBSEQ_RESTORE_A_LITTLE_HP;
                    result = TRUE;
                }
            } else if (Battler_Ability(battleCtx, battler) != ABILITY_MAGIC_GUARD) {
                battleCtx->hpCalcTemp = BattleSystem_Divide(battleCtx->battleMons[battler].maxHP * -1, 8);
                subscript = BATTLE_SUBSEQ_LOSE_HP_FROM_ITEM_WITH_MESSAGE;
                result = TRUE;
            }
            break;
        }

        if (result == TRUE) {
            battleCtx->msgBattlerTemp = battler;
            battleCtx->msgItemTemp = Battler_HeldItem(battleCtx, battler);
            
            LOAD_SUBSEQ(subscript);
            battleCtx->commandNext = battleCtx->command;
            battleCtx->command = BATTLE_CONTROL_EXEC_SCRIPT;
        }
    }

    return result;
}

BOOL BattleSystem_TriggerHeldItemOnStatus(BattleSystem *battleSys, BattleContext *battleCtx, int battler, int *subscript)
{
    BOOL result = FALSE;
    int itemEffect = Battler_HeldItemEffect(battleCtx, battler);
    int itemPower = Battler_HeldItemPower(battleCtx, battler, ITEM_POWER_CHECK_ALL);

    if (battleCtx->battleMons[battler].curHP) {
        switch (itemEffect) {
        case HOLD_EFFECT_HP_RESTORE:
            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / 2) {
                battleCtx->hpCalcTemp = itemPower;
                *subscript = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_HP_PCT_RESTORE:
            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / 2) {
                battleCtx->hpCalcTemp = BattleSystem_Divide(battleCtx->battleMons[battler].maxHP * itemPower, 100);
                *subscript = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_PRZ_RESTORE:
            if (battleCtx->battleMons[battler].status & MON_CONDITION_PARALYSIS) {
                *subscript = BATTLE_SUBSEQ_HELD_ITEM_PRZ_RESTORE;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_SLP_RESTORE:
            if (battleCtx->battleMons[battler].status & MON_CONDITION_SLEEP) {
                *subscript = BATTLE_SUBSEQ_HELD_ITEM_SLP_RESTORE;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_PSN_RESTORE:
            if (battleCtx->battleMons[battler].status & MON_CONDITION_ANY_POISON) {
                *subscript = BATTLE_SUBSEQ_HELD_ITEM_PSN_RESTORE;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_BRN_RESTORE:
            if (battleCtx->battleMons[battler].status & MON_CONDITION_BURN) {
                *subscript = BATTLE_SUBSEQ_HELD_ITEM_BRN_RESTORE;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_FRZ_RESTORE:
            if (battleCtx->battleMons[battler].status & MON_CONDITION_FREEZE) {
                *subscript = BATTLE_SUBSEQ_HELD_ITEM_FRZ_RESTORE;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_PP_RESTORE: {
            int i;
            for (i = 0; i < LEARNED_MOVES_MAX; i++) {
                if (battleCtx->battleMons[battler].moves[i] && battleCtx->battleMons[battler].ppCur[i] == 0) {
                    break;
                }
            }

            if (i != LEARNED_MOVES_MAX) {
                BattleMon_AddVal(&battleCtx->battleMons[battler], BATTLEMON_CUR_PP_1 + i, itemPower);
                BattleMon_CopyToParty(battleSys, battleCtx, battler);
                battleCtx->msgMoveTemp = battleCtx->battleMons[battler].moves[i];
                *subscript = BATTLE_SUBSEQ_HELD_ITEM_PP_RESTORE;
                result = TRUE;
            }

            break;
        }

        case HOLD_EFFECT_CONFUSE_RESTORE:
            if (battleCtx->battleMons[battler].statusVolatile & VOLATILE_CONDITION_CONFUSION) {
                *subscript = BATTLE_SUBSEQ_HELD_ITEM_CNF_RESTORE;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_STATUS_RESTORE:
            if ((battleCtx->battleMons[battler].status & MON_CONDITION_ANY)
                    || (battleCtx->battleMons[battler].statusVolatile & VOLATILE_CONDITION_CONFUSION)) {
                if (battleCtx->battleMons[battler].status & MON_CONDITION_PARALYSIS) {
                    *subscript = BATTLE_SUBSEQ_HELD_ITEM_PRZ_RESTORE;
                }

                if (battleCtx->battleMons[battler].status & MON_CONDITION_SLEEP) {
                    *subscript = BATTLE_SUBSEQ_HELD_ITEM_SLP_RESTORE;
                }

                if (battleCtx->battleMons[battler].status & MON_CONDITION_ANY_POISON) {
                    *subscript = BATTLE_SUBSEQ_HELD_ITEM_PSN_RESTORE;
                }

                if (battleCtx->battleMons[battler].status & MON_CONDITION_BURN) {
                    *subscript = BATTLE_SUBSEQ_HELD_ITEM_BRN_RESTORE;
                }

                if (battleCtx->battleMons[battler].status & MON_CONDITION_FREEZE) {
                    *subscript = BATTLE_SUBSEQ_HELD_ITEM_FRZ_RESTORE;
                }

                if (battleCtx->battleMons[battler].statusVolatile & VOLATILE_CONDITION_CONFUSION) {
                    *subscript = BATTLE_SUBSEQ_HELD_ITEM_CNF_RESTORE;
                }

                if ((battleCtx->battleMons[battler].status & MON_CONDITION_ANY)
                        && (battleCtx->battleMons[battler].statusVolatile & VOLATILE_CONDITION_CONFUSION)) {
                    *subscript = BATTLE_SUBSEQ_HELD_ITEM_MULTI_RESTORE;
                }

                result = TRUE;
            }
            break;

        case HOLD_EFFECT_STATDOWN_RESTORE: {
            for (int i = BATTLE_STAT_HP; i < BATTLE_STAT_MAX; i++) {
                if (battleCtx->battleMons[battler].statBoosts[i] < 6) {
                    battleCtx->battleMons[battler].statBoosts[i] = 6;
                    result = TRUE;
                }
            }

            if (result == TRUE) {
                *subscript = BATTLE_SUBSEQ_HELD_ITEM_STATDOWN_RESTORE;
            }
            break;
        }
            
        case HOLD_EFFECT_HEAL_INFATUATION:
            if (battleCtx->battleMons[battler].statusVolatile & VOLATILE_CONDITION_ATTRACT) {
                battleCtx->msgTemp = MSGCOND_INFATUATION;
                *subscript = BATTLE_SUBSEQ_HELD_ITEM_HEAL_INFATUATION;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_PINCH_ACC_UP:
            if (Battler_Ability(battleCtx, battler) == ABILITY_GLUTTONY) {
                itemPower /= 2;
            }

            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / itemPower) {
                *subscript = BATTLE_SUBSEQ_HELD_ITEM_TEMP_ACC_UP;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_HP_RESTORE_SPICY:
            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / 2) {
                battleCtx->hpCalcTemp = BattleSystem_Divide(battleCtx->battleMons[battler].maxHP, itemPower);
                battleCtx->msgTemp = FLAVOR_SPICY;

                if (Pokemon_GetFlavorAffinityOf(battleCtx->battleMons[battler].personality, FLAVOR_SPICY) == -1) {
                    *subscript = BATTLE_SUBSEQ_HELD_ITEM_DISLIKE_FLAVOR;
                } else {
                    *subscript = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
                }

                result = TRUE;
            }
            break;

        case HOLD_EFFECT_HP_RESTORE_DRY:
            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / 2) {
                battleCtx->hpCalcTemp = BattleSystem_Divide(battleCtx->battleMons[battler].maxHP, itemPower);
                battleCtx->msgTemp = FLAVOR_DRY;

                if (Pokemon_GetFlavorAffinityOf(battleCtx->battleMons[battler].personality, FLAVOR_DRY) == -1) {
                    *subscript = BATTLE_SUBSEQ_HELD_ITEM_DISLIKE_FLAVOR;
                } else {
                    *subscript = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
                }

                result = TRUE;
            }
            break;

        case HOLD_EFFECT_HP_RESTORE_SWEET:
            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / 2) {
                battleCtx->hpCalcTemp = BattleSystem_Divide(battleCtx->battleMons[battler].maxHP, itemPower);
                battleCtx->msgTemp = FLAVOR_SWEET;

                if (Pokemon_GetFlavorAffinityOf(battleCtx->battleMons[battler].personality, FLAVOR_SWEET) == -1) {
                    *subscript = BATTLE_SUBSEQ_HELD_ITEM_DISLIKE_FLAVOR;
                } else {
                    *subscript = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
                }

                result = TRUE;
            }
            break;

        case HOLD_EFFECT_HP_RESTORE_BITTER:
            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / 2) {
                battleCtx->hpCalcTemp = BattleSystem_Divide(battleCtx->battleMons[battler].maxHP, itemPower);
                battleCtx->msgTemp = FLAVOR_BITTER;

                if (Pokemon_GetFlavorAffinityOf(battleCtx->battleMons[battler].personality, FLAVOR_BITTER) == -1) {
                    *subscript = BATTLE_SUBSEQ_HELD_ITEM_DISLIKE_FLAVOR;
                } else {
                    *subscript = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
                }

                result = TRUE;
            }
            break;

        case HOLD_EFFECT_HP_RESTORE_SOUR:
            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / 2) {
                battleCtx->hpCalcTemp = BattleSystem_Divide(battleCtx->battleMons[battler].maxHP, itemPower);
                battleCtx->msgTemp = FLAVOR_SOUR;

                if (Pokemon_GetFlavorAffinityOf(battleCtx->battleMons[battler].personality, FLAVOR_SOUR) == -1) {
                    *subscript = BATTLE_SUBSEQ_HELD_ITEM_DISLIKE_FLAVOR;
                } else {
                    *subscript = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
                }

                result = TRUE;
            }
            break;

        case HOLD_EFFECT_PINCH_ATK_UP:
            if (Battler_Ability(battleCtx, battler) == ABILITY_GLUTTONY) {
                itemPower /= 2;
            }

            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / itemPower
                    && battleCtx->battleMons[battler].statBoosts[BATTLE_STAT_ATTACK] < 12) {
                battleCtx->msgTemp = BATTLE_STAT_ATTACK;
                *subscript = BATTLE_SUBSEQ_HELD_ITEM_RAISE_STAT;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_PINCH_DEF_UP:
            if (Battler_Ability(battleCtx, battler) == ABILITY_GLUTTONY) {
                itemPower /= 2;
            }

            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / itemPower
                    && battleCtx->battleMons[battler].statBoosts[BATTLE_STAT_DEFENSE] < 12) {
                battleCtx->msgTemp = BATTLE_STAT_DEFENSE;
                *subscript = BATTLE_SUBSEQ_HELD_ITEM_RAISE_STAT;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_PINCH_SPEED_UP:
            if (Battler_Ability(battleCtx, battler) == ABILITY_GLUTTONY) {
                itemPower /= 2;
            }

            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / itemPower
                    && battleCtx->battleMons[battler].statBoosts[BATTLE_STAT_SPEED] < 12) {
                battleCtx->msgTemp = BATTLE_STAT_SPEED;
                *subscript = BATTLE_SUBSEQ_HELD_ITEM_RAISE_STAT;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_PINCH_SPATK_UP:
            if (Battler_Ability(battleCtx, battler) == ABILITY_GLUTTONY) {
                itemPower /= 2;
            }

            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / itemPower
                    && battleCtx->battleMons[battler].statBoosts[BATTLE_STAT_SP_ATTACK] < 12) {
                battleCtx->msgTemp = BATTLE_STAT_SP_ATTACK;
                *subscript = BATTLE_SUBSEQ_HELD_ITEM_RAISE_STAT;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_PINCH_SPDEF_UP:
            if (Battler_Ability(battleCtx, battler) == ABILITY_GLUTTONY) {
                itemPower /= 2;
            }

            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / itemPower
                    && battleCtx->battleMons[battler].statBoosts[BATTLE_STAT_SP_DEFENSE] < 12) {
                battleCtx->msgTemp = BATTLE_STAT_SP_DEFENSE;
                *subscript = BATTLE_SUBSEQ_HELD_ITEM_RAISE_STAT;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_PINCH_CRITRATE_UP:
            if (Battler_Ability(battleCtx, battler) == ABILITY_GLUTTONY) {
                itemPower /= 2;
            }

            if (battleCtx->battleMons[battler].curHP <= battleCtx->battleMons[battler].maxHP / itemPower
                    && (battleCtx->battleMons[battler].statusVolatile & VOLATILE_CONDITION_FOCUS_ENERGY) == FALSE) {
                *subscript = BATTLE_SUBSEQ_HELD_ITEM_RAISE_CRIT;
                result = TRUE;
            }
            break;

        case HOLD_EFFECT_PINCH_RANDOM_UP:
            if (Battler_Ability(battleCtx, battler) == ABILITY_GLUTTONY) {
                itemPower /= 2;
            }

            if (battleCtx->battleMons[battler].curHP <= (battleCtx->battleMons[battler].maxHP / itemPower)) {
                int i;
                for (i = 0; i < 5; i++) {
                    if (battleCtx->battleMons[battler].statBoosts[BATTLE_STAT_ATTACK + i] < 12) {
                        break;
                    }
                }

                if (i != 5) {
                    do {
                        i = BattleSystem_RandNext(battleSys) % 5;
                    } while (battleCtx->battleMons[battler].statBoosts[BATTLE_STAT_ATTACK + i] == 12);

                    battleCtx->msgTemp = BATTLE_STAT_ATTACK + i;
                    *subscript = BATTLE_SUBSEQ_HELD_ITEM_SHARPLY_RAISE_STAT;
                    result = TRUE;
                }
            }
            break;
        }

        if (result == TRUE) {
            battleCtx->msgItemTemp = Battler_HeldItem(battleCtx, battler);
        }
    }

    return result;
}

BOOL BattleSystem_TriggerDetrimentalHeldItem(BattleSystem *battleSys, BattleContext *battleCtx, int battler)
{
    BOOL result = FALSE;
    int subscript;
    int itemEffect = Battler_HeldItemEffect(battleCtx, battler);
    int itemPower = Battler_HeldItemPower(battleCtx, battler, 0);

    if (battleCtx->battleMons[battler].curHP) {
        switch (itemEffect) {
        case HOLD_EFFECT_PSN_USER:
            battleCtx->sideEffectMon = battler;
            battleCtx->sideEffectType = SIDE_EFFECT_TYPE_HELD_ITEM;
            subscript = BATTLE_SUBSEQ_BADLY_POISON;
            result = TRUE;
            break;
            
        case HOLD_EFFECT_BRN_USER:
            battleCtx->sideEffectMon = battler;
            battleCtx->sideEffectType = SIDE_EFFECT_TYPE_HELD_ITEM;
            subscript = BATTLE_SUBSEQ_BURN;
            result = TRUE;
            break;

        case HOLD_EFFECT_DMG_USER_CONTACT_XFR:
            if (Battler_Ability(battleCtx, battler) != ABILITY_MAGIC_GUARD) {
                battleCtx->hpCalcTemp = BattleSystem_Divide(battleCtx->battleMons[battler].maxHP * -1, itemPower);
                subscript = BATTLE_SUBSEQ_LOSE_HP_FROM_ITEM_WITH_MESSAGE;
                result = TRUE;
            }
            break;
        }

        if (result == TRUE) {
            battleCtx->msgBattlerTemp = battler;
            battleCtx->msgItemTemp = Battler_HeldItem(battleCtx, battler);

            LOAD_SUBSEQ(subscript);
            battleCtx->commandNext = battleCtx->command;
            battleCtx->command = BATTLE_CONTROL_EXEC_SCRIPT;
        }
    }

    return result;
}

u16 Battler_HeldItem(BattleContext *battleCtx, int battler)
{
    if (Battler_Ability(battleCtx, battler) == ABILITY_KLUTZ) {
        return ITEM_NONE;
    }

    if (battleCtx->battleMons[battler].moveEffectsData.embargoTurns) {
        return ITEM_NONE;
    }

    return battleCtx->battleMons[battler].heldItem;
}

BOOL Battler_MovedThisTurn(BattleContext *battleCtx, int battler)
{
    return battleCtx->battlerActions[battler][BATTLE_ACTION_PICK_COMMAND] == BATTLE_CONTROL_MOVE_END;
}

BOOL BattleSystem_TriggerHeldItemOnHit(BattleSystem *battleSys, BattleContext *battleCtx, int *subscript)
{
    BOOL result = FALSE;

    if (battleCtx->defender == BATTLER_NONE) {
        return result;
    }

    if (Battler_SubstituteWasHit(battleCtx, battleCtx->defender) == TRUE) {
        return result;
    }

    int itemEffect = Battler_HeldItemEffect(battleCtx, battleCtx->defender);
    int itemPower = Battler_HeldItemPower(battleCtx, battleCtx->defender, 0);
    int side = Battler_Side(battleSys, battleCtx->attacker);

    switch (itemEffect) {
    case HOLD_EFFECT_DMG_USER_CONTACT_XFR:
        if (ATTACKING_MON.curHP
                && ATTACKING_MON.heldItem == ITEM_NONE
                && (battleCtx->sideConditions[side].knockedOffItemsMask & FlagIndex(battleCtx->selectedPartySlot[battleCtx->attacker])) == FALSE
                && battleCtx->moveCur != MOVE_KNOCK_OFF
                && (DEFENDER_SELF_TURN_FLAGS.physicalDamageTaken || DEFENDER_SELF_TURN_FLAGS.specialDamageTaken)
                && (battleCtx->battleStatusMask2 & SYSCTL_UTURN_ACTIVE) == FALSE
                && (CURRENT_MOVE_DATA.flags & MOVE_FLAG_MAKES_CONTACT)) {
            *subscript = BATTLE_SUBSEQ_TRANSFER_STICKY_BARB;
            result = TRUE;
        }
        break;

    case HOLD_EFFECT_RECOIL_PHYSICAL:
        if (ATTACKING_MON.curHP
                && Battler_Ability(battleCtx, battleCtx->attacker) != ABILITY_MAGIC_GUARD
                && (battleCtx->battleStatusMask2 & SYSCTL_UTURN_ACTIVE) == FALSE
                && DEFENDER_SELF_TURN_FLAGS.physicalDamageTaken) {
            battleCtx->hpCalcTemp = BattleSystem_Divide(ATTACKING_MON.maxHP * -1, itemPower);
            *subscript = BATTLE_SUBSEQ_HELD_ITEM_RECOIL_WHEN_HIT;
            result = TRUE;
        }
        break;

    case HOLD_EFFECT_RECOIL_SPECIAL:
        if (ATTACKING_MON.curHP
                && Battler_Ability(battleCtx, battleCtx->attacker) != ABILITY_MAGIC_GUARD
                && DEFENDER_SELF_TURN_FLAGS.specialDamageTaken) {
            battleCtx->hpCalcTemp = BattleSystem_Divide(ATTACKING_MON.maxHP * -1, itemPower);
            *subscript = BATTLE_SUBSEQ_HELD_ITEM_RECOIL_WHEN_HIT;
            result = TRUE;
        }
        break;

    case HOLD_EFFECT_HP_RESTORE_SE:
        if (DEFENDING_MON.curHP && (battleCtx->moveStatusFlags & MOVE_STATUS_SUPER_EFFECTIVE)) {
            battleCtx->hpCalcTemp = BattleSystem_Divide(DEFENDING_MON.maxHP, itemPower);
            *subscript = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
            battleCtx->msgBattlerTemp = battleCtx->defender;
            battleCtx->msgItemTemp = battleCtx->battleMons[battleCtx->defender].heldItem;
            result = TRUE;
        }
        break;

    default:
        break;
    }

    return result;
}

s32 Battler_HeldItemEffect(BattleContext *battleCtx, int battler)
{
    u16 item = Battler_HeldItem(battleCtx, battler);
    return BattleSystem_GetItemData(battleCtx, item, ITEM_PARAM_HOLD_EFFECT);
}

s32 Battler_HeldItemPower(BattleContext *battleCtx, int battler, enum HeldItemPowerOp opcode)
{
    u16 item;
    switch (opcode) {
    case ITEM_POWER_CHECK_ALL:
        item = Battler_HeldItem(battleCtx, battler);
        break;

    case ITEM_POWER_CHECK_EMBARGO:
        if (battleCtx->battleMons[battler].moveEffectsData.embargoTurns) {
            return 0;
        }

    case ITEM_POWER_CHECK_NONE:
        item = battleCtx->battleMons[battler].heldItem;
        break;
    }

    return BattleSystem_GetItemData(battleCtx, item, ITEM_PARAM_HOLD_EFFECT_PARAM);
}

s32 Battler_NaturalGiftPower(BattleContext *battleCtx, int battler)
{
    u16 item = Battler_HeldItem(battleCtx, battler);
    return BattleSystem_GetItemData(battleCtx, item, ITEM_PARAM_NATURAL_GIFT_POWER);
}

s32 Battler_NaturalGiftType(BattleContext *battleCtx, int battler)
{
    u16 item = Battler_HeldItem(battleCtx, battler);
    return BattleSystem_GetItemData(battleCtx, item, ITEM_PARAM_NATURAL_GIFT_TYPE);
}

s32 Battler_ItemPluckEffect(BattleContext *battleCtx, int battler)
{
    return BattleSystem_GetItemData(battleCtx, battleCtx->battleMons[battler].heldItem, ITEM_PARAM_PLUCK_EFFECT);
}

s32 Battler_ItemFlingEffect(BattleContext *battleCtx, int battler)
{
    if (battleCtx->battleMons[battler].moveEffectsData.embargoTurns) {
        return FLING_EFFECT_NONE;
    }

    return BattleSystem_GetItemData(battleCtx, battleCtx->battleMons[battler].heldItem, ITEM_PARAM_FLING_EFFECT);
}

s32 Battler_ItemFlingPower(BattleContext *battleCtx, int battler)
{
    if (battleCtx->battleMons[battler].moveEffectsData.embargoTurns) {
        return 0;
    }

    return BattleSystem_GetItemData(battleCtx, battleCtx->battleMons[battler].heldItem, ITEM_PARAM_FLING_POWER);
}

int BattleSystem_CanSwitch (BattleSystem *battleSys, BattleContext *battleCtx, int battler)
{
    int v0;

    v0 = 0;

    if (Battler_HeldItemEffect(battleCtx, battler) == 123) {
        return 0;
    }

    if ((battleCtx->battleMons[battler].statusVolatile & (0xe000 | 0x4000000)) || (battleCtx->battleMons[battler].moveEffectsMask & 0x400)) {
        v0 = 1;
    }

    if (((Battler_Ability(battleCtx, battler) != 23) && (BattleSystem_CountAbility(battleSys, battleCtx, 3, battler, 23))) || (((BattleMon_Get(battleCtx, battler, 27, NULL) == 8) || (BattleMon_Get(battleCtx, battler, 28, NULL) == 8)) && (BattleSystem_CountAbility(battleSys, battleCtx, 3, battler, 42)))) {
        v0 = 1;
    }

    if ((((Battler_Ability(battleCtx, battler) != 26) && (battleCtx->battleMons[battler].moveEffectsData.magnetRiseTurns == 0) && (BattleMon_Get(battleCtx, battler, 27, NULL) != 2) && (BattleMon_Get(battleCtx, battler, 28, NULL) != 2)) || (Battler_HeldItemEffect(battleCtx, battler) == 106) || (battleCtx->fieldConditionsMask & 0x7000)) && (BattleSystem_CountAbility(battleSys, battleCtx, 3, battler, 71))) {
        v0 = 1;
    }

    return v0;
}

BOOL BattleSystem_PluckBerry(BattleSystem *battleSys, BattleContext *battleCtx, int battler)
{
    BOOL result = FALSE;
    int nextSeq = 0;
    int effect = Battler_ItemPluckEffect(battleCtx, battler);
    int power = Battler_HeldItemPower(battleCtx, battler, 1);

    if (Battler_SubstituteWasHit(battleCtx, battleCtx->defender) == TRUE) {
        return result;
    }

    switch (effect) {
    case PLUCK_EFFECT_HP_RESTORE:
        if (ATTACKING_MON.curHP != ATTACKING_MON.maxHP) {
            battleCtx->hpCalcTemp = power;
            nextSeq = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
        }

        result = TRUE;
        break;

    case PLUCK_EFFECT_HP_PCT_RESTORE:
        if (ATTACKING_MON.curHP != ATTACKING_MON.maxHP) {
            battleCtx->hpCalcTemp = BattleSystem_Divide(ATTACKING_MON.maxHP * power, 100);
            nextSeq = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
        }

        result = TRUE;
        break;

    case PLUCK_EFFECT_PRZ_RESTORE:
        if (ATTACKING_MON.status & MON_CONDITION_PARALYSIS) {
            nextSeq = BATTLE_SUBSEQ_HELD_ITEM_PRZ_RESTORE;
        }

        result = TRUE;
        break;

    case PLUCK_EFFECT_SLP_RESTORE:
        if (ATTACKING_MON.status & MON_CONDITION_SLEEP) {
            nextSeq = BATTLE_SUBSEQ_HELD_ITEM_SLP_RESTORE;
        }

        result = TRUE;
        break;

    case PLUCK_EFFECT_PSN_RESTORE:
        if (ATTACKING_MON.status & MON_CONDITION_ANY_POISON) {
            nextSeq = BATTLE_SUBSEQ_HELD_ITEM_PSN_RESTORE;
        }

        result = TRUE;
        break;

    case PLUCK_EFFECT_BRN_RESTORE:
        if (ATTACKING_MON.status & MON_CONDITION_BURN) {
            nextSeq = BATTLE_SUBSEQ_HELD_ITEM_BRN_RESTORE;
        }

        result = TRUE;
        break;

    case PLUCK_EFFECT_FRZ_RESTORE:
        if (ATTACKING_MON.status & MON_CONDITION_FREEZE) {
            nextSeq = BATTLE_SUBSEQ_HELD_ITEM_FRZ_RESTORE;
        }

        result = TRUE;
        break;

    case PLUCK_EFFECT_PP_RESTORE: {
        int diff, i, maxDiff, slot; // must declare in this order to match.

        // Find the move that has the highest difference between its current
        // and maximum PP.
        maxDiff = 0;
        for (i = 0; i < LEARNED_MOVES_MAX; i++) {
            if (ATTACKING_MON.moves[i]) {
                diff = MoveTable_CalcMaxPP(ATTACKING_MON.moves[i], ATTACKING_MON.ppUps[i]) - ATTACKING_MON.ppCur[i];

                if (diff > maxDiff) {
                    maxDiff = diff;
                    slot = i;
                }
            }
        }

        BattleMon_AddVal(&ATTACKING_MON, BATTLEMON_CUR_PP_1 + slot, power);
        BattleMon_CopyToParty(battleSys, battleCtx, battleCtx->attacker);
        battleCtx->msgMoveTemp = ATTACKING_MON.moves[slot];
        nextSeq = BATTLE_SUBSEQ_HELD_ITEM_PP_RESTORE;

        result = TRUE;
        break;
    }

    case PLUCK_EFFECT_CNF_RESTORE:
        if (ATTACKING_MON.statusVolatile & VOLATILE_CONDITION_CONFUSION) {
            nextSeq = BATTLE_SUBSEQ_HELD_ITEM_CNF_RESTORE;
        }

        result = TRUE;
        break;

    case PLUCK_EFFECT_ALL_RESTORE:
        if ((ATTACKING_MON.status & MON_CONDITION_ANY) || (ATTACKING_MON.statusVolatile & VOLATILE_CONDITION_CONFUSION)) {
            if (ATTACKING_MON.status & MON_CONDITION_PARALYSIS) {
                nextSeq = BATTLE_SUBSEQ_HELD_ITEM_PRZ_RESTORE;
            }

            if (ATTACKING_MON.status & MON_CONDITION_SLEEP) {
                nextSeq = BATTLE_SUBSEQ_HELD_ITEM_SLP_RESTORE;
            }

            if (ATTACKING_MON.status & MON_CONDITION_ANY_POISON) {
                nextSeq = BATTLE_SUBSEQ_HELD_ITEM_PSN_RESTORE;
            }

            if (ATTACKING_MON.status & MON_CONDITION_BURN) {
                nextSeq = BATTLE_SUBSEQ_HELD_ITEM_BRN_RESTORE;
            }

            if (ATTACKING_MON.status & MON_CONDITION_FREEZE) {
                nextSeq = BATTLE_SUBSEQ_HELD_ITEM_FRZ_RESTORE;
            }

            if (ATTACKING_MON.statusVolatile & VOLATILE_CONDITION_CONFUSION) {
                nextSeq = BATTLE_SUBSEQ_HELD_ITEM_CNF_RESTORE;
            }

            if ((ATTACKING_MON.status & MON_CONDITION_ANY)
                    && (ATTACKING_MON.statusVolatile & VOLATILE_CONDITION_CONFUSION)) {
                nextSeq = BATTLE_SUBSEQ_HELD_ITEM_MULTI_RESTORE;
            }
        }

        result = TRUE;
        break;

    case PLUCK_EFFECT_HP_RESTORE_SPICY:
        if (ATTACKING_MON.curHP != ATTACKING_MON.maxHP) {
            battleCtx->hpCalcTemp = BattleSystem_Divide(ATTACKING_MON.maxHP, power);
            battleCtx->msgTemp = FLAVOR_SPICY;

            if (Pokemon_GetFlavorAffinityOf(ATTACKING_MON.personality, FLAVOR_SPICY) == -1) {
                nextSeq = BATTLE_SUBSEQ_HELD_ITEM_DISLIKE_FLAVOR;
            } else {
                nextSeq = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
            }
        }

        result = TRUE;
        break;

    case PLUCK_EFFECT_HP_RESTORE_DRY:
        if (ATTACKING_MON.curHP != ATTACKING_MON.maxHP) {
            battleCtx->hpCalcTemp = BattleSystem_Divide(ATTACKING_MON.maxHP, power);
            battleCtx->msgTemp = FLAVOR_DRY;

            if (Pokemon_GetFlavorAffinityOf(ATTACKING_MON.personality, FLAVOR_DRY) == -1) {
                nextSeq = BATTLE_SUBSEQ_HELD_ITEM_DISLIKE_FLAVOR;
            } else {
                nextSeq = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
            }
        }

        result = TRUE;
        break;

    case PLUCK_EFFECT_HP_RESTORE_SWEET:
        if (ATTACKING_MON.curHP != ATTACKING_MON.maxHP) {
            battleCtx->hpCalcTemp = BattleSystem_Divide(ATTACKING_MON.maxHP, power);
            battleCtx->msgTemp = FLAVOR_SWEET;

            if (Pokemon_GetFlavorAffinityOf(ATTACKING_MON.personality, FLAVOR_SWEET) == -1) {
                nextSeq = BATTLE_SUBSEQ_HELD_ITEM_DISLIKE_FLAVOR;
            } else {
                nextSeq = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
            }
        }

        result = TRUE;
        break;

    case PLUCK_EFFECT_HP_RESTORE_BITTER:
        if (ATTACKING_MON.curHP != ATTACKING_MON.maxHP) {
            battleCtx->hpCalcTemp = BattleSystem_Divide(ATTACKING_MON.maxHP, power);
            battleCtx->msgTemp = FLAVOR_BITTER;

            if (Pokemon_GetFlavorAffinityOf(ATTACKING_MON.personality, FLAVOR_BITTER) == -1) {
                nextSeq = BATTLE_SUBSEQ_HELD_ITEM_DISLIKE_FLAVOR;
            } else {
                nextSeq = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
            }
        }

        result = TRUE;
        break;

    case PLUCK_EFFECT_HP_RESTORE_SOUR:
        if (ATTACKING_MON.curHP != ATTACKING_MON.maxHP) {
            battleCtx->hpCalcTemp = BattleSystem_Divide(ATTACKING_MON.maxHP, power);
            battleCtx->msgTemp = FLAVOR_SOUR;

            if (Pokemon_GetFlavorAffinityOf(ATTACKING_MON.personality, FLAVOR_SOUR) == -1) {
                nextSeq = BATTLE_SUBSEQ_HELD_ITEM_DISLIKE_FLAVOR;
            } else {
                nextSeq = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
            }
        }

        result = TRUE;
        break;

    case PLUCK_EFFECT_ATK_UP:
        if (ATTACKING_MON.statBoosts[BATTLE_STAT_ATTACK] < 12) {
            battleCtx->msgTemp = BATTLE_STAT_ATTACK;
            nextSeq = BATTLE_SUBSEQ_HELD_ITEM_RAISE_STAT;
        }

        result = TRUE;
        break;

    case PLUCK_EFFECT_DEF_UP:
        if (ATTACKING_MON.statBoosts[BATTLE_STAT_DEFENSE] < 12) {
            battleCtx->msgTemp = BATTLE_STAT_DEFENSE;
            nextSeq = BATTLE_SUBSEQ_HELD_ITEM_RAISE_STAT;
        }

        result = TRUE;
        break;

    case PLUCK_EFFECT_SPEED_UP:
        if (ATTACKING_MON.statBoosts[BATTLE_STAT_SPEED] < 12) {
            battleCtx->msgTemp = BATTLE_STAT_SPEED;
            nextSeq = BATTLE_SUBSEQ_HELD_ITEM_RAISE_STAT;
        }

        result = TRUE;
        break;

    case PLUCK_EFFECT_SPATK_UP:
        if (ATTACKING_MON.statBoosts[BATTLE_STAT_SP_ATTACK] < 12) {
            battleCtx->msgTemp = BATTLE_STAT_SP_ATTACK;
            nextSeq = BATTLE_SUBSEQ_HELD_ITEM_RAISE_STAT;
        }

        result = TRUE;
        break;

    case PLUCK_EFFECT_SPDEF_UP:
        if (ATTACKING_MON.statBoosts[BATTLE_STAT_SP_DEFENSE] < 12) {
            battleCtx->msgTemp = BATTLE_STAT_SP_DEFENSE;
            nextSeq = BATTLE_SUBSEQ_HELD_ITEM_RAISE_STAT;
        }

        result = TRUE;
        break;

    case PLUCK_EFFECT_RANDOM_UP2: {
        int stat;
        for (stat = 0; stat < 5; stat++) {
            if (ATTACKING_MON.statBoosts[BATTLE_STAT_ATTACK + stat] < 12) {
                break;
            }
        }

        if (stat != 5) {
            do {
                stat = BattleSystem_RandNext(battleSys) % 5;
            } while (ATTACKING_MON.statBoosts[BATTLE_STAT_ATTACK + stat] == 12);

            battleCtx->msgTemp = BATTLE_STAT_ATTACK + stat;
            nextSeq = BATTLE_SUBSEQ_HELD_ITEM_SHARPLY_RAISE_STAT;
        }

        result = TRUE;
        break;
    }

    case PLUCK_EFFECT_CRIT_UP:
        if ((ATTACKING_MON.statusVolatile & VOLATILE_CONDITION_FOCUS_ENERGY) == FALSE) {
            nextSeq = BATTLE_SUBSEQ_HELD_ITEM_RAISE_CRIT;
        }

        result = TRUE;
        break;

    case PLUCK_EFFECT_TEMP_ACC_UP:
        nextSeq = BATTLE_SUBSEQ_HELD_ITEM_TEMP_ACC_UP;
        result = TRUE;
        break;

    default:
        if (Item_IsBerry(battleCtx->battleMons[battler].heldItem) == TRUE) {
            result = TRUE;
        }
        break;
    }

    if (result == TRUE) {
        if (Battler_Ability(battleCtx, battleCtx->attacker) == ABILITY_KLUTZ
                || (ATTACKING_MON.moveEffectsMask & MOVE_EFFECT_EMBARGO)) {
            battleCtx->scriptTemp = 0;
        } else {
            battleCtx->scriptTemp = nextSeq;
        }

        battleCtx->msgItemTemp = battleCtx->battleMons[battler].heldItem;
        ATTACKER_SELF_TURN_FLAGS.statusFlags |= SELF_TURN_FLAG_PLUCK_BERRY;
    }

    return result;
}

BOOL BattleSystem_FlingItem(BattleSystem *battleSys, BattleContext *battleCtx, int battler)
{
    int effect = Battler_ItemFlingEffect(battleCtx, battler);
    int effectPower = Battler_HeldItemPower(battleCtx, battler, ITEM_PARAM_HOLD_EFFECT_PARAM);

    battleCtx->movePower = Battler_ItemFlingPower(battleCtx, battler);
    battleCtx->flingScript = 0;
    battleCtx->sideEffectType = SIDE_EFFECT_TYPE_NONE;

    if (battleCtx->movePower == 0) {
        return FALSE;
    }

    switch (effect) {
    case FLING_EFFECT_HP_RESTORE:
        battleCtx->flingTemp = effectPower;
        battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
        break;

    case FLING_EFFECT_HP_PCT_RESTORE:
        battleCtx->flingTemp = BattleSystem_Divide(DEFENDING_MON.maxHP * effectPower, 100);
        battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
        break;

    case FLING_EFFECT_PRZ_RESTORE:
        if (DEFENDING_MON.status & MON_CONDITION_PARALYSIS) {
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_PRZ_RESTORE;
        }
        break;

    case FLING_EFFECT_SLP_RESTORE:
        if (DEFENDING_MON.status & MON_CONDITION_SLEEP) {
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_SLP_RESTORE;
        }
        break;

    case FLING_EFFECT_PSN_RESTORE:
        if (DEFENDING_MON.status & MON_CONDITION_ANY_POISON) {
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_PSN_RESTORE;
        }
        break;

    case FLING_EFFECT_BRN_RESTORE:
        if (DEFENDING_MON.status & MON_CONDITION_BURN) {
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_BRN_RESTORE;
        }
        break;

    case FLING_EFFECT_FRZ_RESTORE:
        if (DEFENDING_MON.status & MON_CONDITION_FREEZE) {
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_FRZ_RESTORE;
        }
        break;

    case FLING_EFFECT_PP_RESTORE: {
        int diff, i, maxDiff, slot; // must declare in this order to match.

        // Find the move that has the highest difference between its current
        // and maximum PP.
        maxDiff = 0;
        for (i = 0; i < LEARNED_MOVES_MAX; i++) {
            if (DEFENDING_MON.moves[i]) {
                diff = MoveTable_CalcMaxPP(DEFENDING_MON.moves[i], DEFENDING_MON.ppUps[i]) - DEFENDING_MON.ppCur[i];

                if (diff > maxDiff) {
                    maxDiff = diff;
                    slot = i;
                }
            }
        }

        if (maxDiff) {
            BattleMon_AddVal(&DEFENDING_MON, BATTLEMON_CUR_PP_1 + slot, effectPower);
            BattleMon_CopyToParty(battleSys, battleCtx, battleCtx->defender);
            battleCtx->msgMoveTemp = DEFENDING_MON.moves[slot];
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_PP_RESTORE;
        }

        break;
    }

    case FLING_EFFECT_CNF_RESTORE:
        if (DEFENDING_MON.statusVolatile & VOLATILE_CONDITION_CONFUSION) {
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_CNF_RESTORE;
        }
        break;

    case FLING_EFFECT_ALL_RESTORE:
        if ((DEFENDING_MON.status & MON_CONDITION_ANY) || (DEFENDING_MON.statusVolatile & VOLATILE_CONDITION_CONFUSION)) {
            if (DEFENDING_MON.status & MON_CONDITION_PARALYSIS) {
                battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_PRZ_RESTORE;
            }

            if (DEFENDING_MON.status & MON_CONDITION_SLEEP) {
                battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_SLP_RESTORE;
            }

            if (DEFENDING_MON.status & MON_CONDITION_ANY_POISON) {
                battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_PSN_RESTORE;
            }

            if (DEFENDING_MON.status & MON_CONDITION_BURN) {
                battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_BRN_RESTORE;
            }

            if (DEFENDING_MON.status & MON_CONDITION_FREEZE) {
                battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_FRZ_RESTORE;
            }

            if (DEFENDING_MON.statusVolatile & VOLATILE_CONDITION_CONFUSION) {
                battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_CNF_RESTORE;
            }

            if ((DEFENDING_MON.status & MON_CONDITION_ANY)
                    && (DEFENDING_MON.statusVolatile & VOLATILE_CONDITION_CONFUSION)) {
                battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_MULTI_RESTORE;
            }
        }
        break;

    case FLING_EFFECT_HP_RESTORE_SPICY:
        battleCtx->flingTemp = BattleSystem_Divide(DEFENDING_MON.maxHP, effectPower);
        battleCtx->msgTemp = FLAVOR_SPICY;

        if (Pokemon_GetFlavorAffinityOf(DEFENDING_MON.personality, FLAVOR_SPICY) == -1) {
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_DISLIKE_FLAVOR;
        } else {
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
        }
        break;

    case FLING_EFFECT_HP_RESTORE_DRY:
        battleCtx->flingTemp = BattleSystem_Divide(DEFENDING_MON.maxHP, effectPower);
        battleCtx->msgTemp = FLAVOR_DRY;

        if (Pokemon_GetFlavorAffinityOf(DEFENDING_MON.personality, FLAVOR_DRY) == -1) {
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_DISLIKE_FLAVOR;
        } else {
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
        }
        break;

    case FLING_EFFECT_HP_RESTORE_SWEET:
        battleCtx->flingTemp = BattleSystem_Divide(DEFENDING_MON.maxHP, effectPower);
        battleCtx->msgTemp = FLAVOR_SWEET;

        if (Pokemon_GetFlavorAffinityOf(DEFENDING_MON.personality, FLAVOR_SWEET) == -1) {
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_DISLIKE_FLAVOR;
        } else {
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
        }
        break;

    case FLING_EFFECT_HP_RESTORE_BITTER:
        battleCtx->flingTemp = BattleSystem_Divide(DEFENDING_MON.maxHP, effectPower);
        battleCtx->msgTemp = FLAVOR_BITTER;

        if (Pokemon_GetFlavorAffinityOf(DEFENDING_MON.personality, FLAVOR_BITTER) == -1) {
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_DISLIKE_FLAVOR;
        } else {
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
        }
        break;

    case FLING_EFFECT_HP_RESTORE_SOUR:
        battleCtx->flingTemp = BattleSystem_Divide(DEFENDING_MON.maxHP, effectPower);
        battleCtx->msgTemp = FLAVOR_SOUR;

        if (Pokemon_GetFlavorAffinityOf(DEFENDING_MON.personality, FLAVOR_SOUR) == -1) {
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_DISLIKE_FLAVOR;
        } else {
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_HP_RESTORE;
        }
        break;

    case FLING_EFFECT_STATDOWN_RESTORE:
        for (int i = BATTLE_STAT_HP; i < BATTLE_STAT_MAX; i++) {
            if (DEFENDING_MON.statBoosts[i] < 6) {
                DEFENDING_MON.statBoosts[i] = 6;
                battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_STATDOWN_RESTORE;
            }
        }
        break;

    case FLING_EFFECT_HEAL_INFATUATION:
        if (DEFENDING_MON.statusVolatile & VOLATILE_CONDITION_ATTRACT) {
            battleCtx->msgTemp = MSGCOND_INFATUATION;
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_HEAL_INFATUATION;
        }
        break;

    case FLING_EFFECT_FLINCH:
        battleCtx->sideEffectMon = battler;
        battleCtx->sideEffectType = SIDE_EFFECT_TYPE_INDIRECT;
        battleCtx->flingScript = BATTLE_SUBSEQ_FLINCH_MON;
        break;

    case FLING_EFFECT_PARALYZE:
        battleCtx->sideEffectMon = battler;
        battleCtx->sideEffectType = SIDE_EFFECT_TYPE_INDIRECT;
        battleCtx->flingScript = BATTLE_SUBSEQ_PARALYZE;
        break;

    case FLING_EFFECT_POISON:
        battleCtx->sideEffectMon = battler;
        battleCtx->sideEffectType = SIDE_EFFECT_TYPE_INDIRECT;
        battleCtx->flingScript = BATTLE_SUBSEQ_POISON;
        break;

    case FLING_EFFECT_BADLY_POISON:
        battleCtx->sideEffectMon = battler;
        battleCtx->sideEffectType = SIDE_EFFECT_TYPE_INDIRECT;
        battleCtx->flingScript = BATTLE_SUBSEQ_BADLY_POISON;
        break;

    case FLING_EFFECT_BURN:
        battleCtx->sideEffectMon = battler;
        battleCtx->sideEffectType = SIDE_EFFECT_TYPE_INDIRECT;
        battleCtx->flingScript = BATTLE_SUBSEQ_BURN;
        break;

    case FLING_EFFECT_ATK_UP:
        if (DEFENDING_MON.statBoosts[BATTLE_STAT_ATTACK] < 12) {
            battleCtx->msgTemp = BATTLE_STAT_ATTACK;
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_RAISE_STAT;
        }
        break;

    case FLING_EFFECT_DEF_UP:
        if (DEFENDING_MON.statBoosts[BATTLE_STAT_DEFENSE] < 12) {
            battleCtx->msgTemp = BATTLE_STAT_DEFENSE;
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_RAISE_STAT;
        }
        break;

    case FLING_EFFECT_SPEED_UP:
        if (DEFENDING_MON.statBoosts[BATTLE_STAT_SPEED] < 12) {
            battleCtx->msgTemp = BATTLE_STAT_SPEED;
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_RAISE_STAT;
        }
        break;

    case FLING_EFFECT_SPATK_UP:
        if (DEFENDING_MON.statBoosts[BATTLE_STAT_SP_ATTACK] < 12) {
            battleCtx->msgTemp = BATTLE_STAT_SP_ATTACK;
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_RAISE_STAT;
        }
        break;

    case FLING_EFFECT_SPDEF_UP:
        if (DEFENDING_MON.statBoosts[BATTLE_STAT_SP_DEFENSE] < 12) {
            battleCtx->msgTemp = BATTLE_STAT_SP_DEFENSE;
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_RAISE_STAT;
        }
        break;

    case FLING_EFFECT_RANDOM_UP2: {
        int stat;

        for (stat = 0; stat < 5; stat++) {
            if (DEFENDING_MON.statBoosts[BATTLE_STAT_ATTACK + stat] < 12) {
                break;
            }
        }

        if (stat != 5) {
            do {
                stat = BattleSystem_RandNext(battleSys) % 5;
            } while (DEFENDING_MON.statBoosts[BATTLE_STAT_ATTACK + stat] == 12);

            battleCtx->msgTemp = BATTLE_STAT_ATTACK + stat;
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_SHARPLY_RAISE_STAT;
        }

        break;
    }

    case FLING_EFFECT_CRIT_UP:
        if ((DEFENDING_MON.statusVolatile & VOLATILE_CONDITION_FOCUS_ENERGY) == FALSE) {
            battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_RAISE_CRIT;
        }
        break;

    case FLING_EFFECT_TEMP_ACC_UP:
        battleCtx->flingScript = BATTLE_SUBSEQ_HELD_ITEM_TEMP_ACC_UP;
        break;

    default:
        break;
    }

    if (DEFENDING_MON.moveEffectsMask & MOVE_EFFECT_EMBARGO) {
        battleCtx->flingScript = 0;
    } else {
        battleCtx->msgItemTemp = battleCtx->battleMons[battler].heldItem;

        if (battleCtx->sideEffectType == SIDE_EFFECT_TYPE_NONE && battleCtx->flingScript) {
            ATTACKER_SELF_TURN_FLAGS.statusFlags |= SELF_TURN_FLAG_PLUCK_BERRY;
        }

        battleCtx->msgBattlerTemp = battleCtx->defender;
    }

    return TRUE;
}

void BattleSystem_UpdateMetronomeCount (BattleSystem * param0, BattleContext * param1)
{
    if (Battler_HeldItemEffect(param1, param1->attacker) == 105) {
        if (((param1->battleMons[param1->attacker].statusVolatile & 0xc00) == 0) && ((param1->battleMons[param1->attacker].statusVolatile & 0x70) == 0) && ((param1->battleStatusMask & 0x200) == 0) && ((param1->battleMons[param1->attacker].statusVolatile & 0x1000) == 0)) {
            if (param1->metronomeMove[param1->attacker] == param1->moveTemp) {
                if (param1->battleMons[param1->attacker].moveEffectsData.metronomeTurns < 10) {
                    param1->battleMons[param1->attacker].moveEffectsData.metronomeTurns++;
                }
            } else {
                param1->battleMons[param1->attacker].moveEffectsData.metronomeTurns = 0;
                param1->metronomeMove[param1->attacker] = param1->moveTemp;
            }
        }
    } else {
        param1->battleMons[param1->attacker].moveEffectsData.metronomeTurns = 0;
    }
}

void BattleSystem_VerifyMetronomeCount (BattleSystem * param0, BattleContext * param1)
{
    if (Battler_HeldItemEffect(param1, param1->attacker) == 105) {
        if ((param1->moveStatusFlags & ((1 | 8 | 64 | 2048 | 4096 | 16384 | 32768 | 65536 | 131072 | 262144 | 524288 | 1048576) | 512 | 0x80000000)) && (param1->metronomeMove[param1->attacker] == param1->moveTemp) && (param1->battleMons[param1->attacker].moveEffectsData.metronomeTurns) && (param1->selfTurnFlags[param1->attacker].repeatedMoveCount == 0) && ((param1->battleMons[param1->attacker].statusVolatile & 0xc00) == 0) && ((param1->battleMons[param1->attacker].statusVolatile & 0x70) == 0) && ((param1->battleStatusMask & 0x200) == 0) && ((param1->battleMons[param1->attacker].statusVolatile & 0x1000) == 0)) {
            param1->battleMons[param1->attacker].moveEffectsData.metronomeTurns--;
        }
    } else {
        param1->battleMons[param1->attacker].moveEffectsData.metronomeTurns = 0;
    }
}

int ov16_022599D0 (BattleContext * param0, int param1, int param2, int param3)
{
    int v0;
    int v1;
    int v2;

    if ((param3 == 1) && ((param2 == 2) || (param2 == 3))) {
        v2 = 1;
    } else {
        v2 = 0;
    }

    v0 = 0;

    if (v2 == 1) {
        v0 = 0;
    }

    v1 = sub_0208C104(param0->battleMons[param1].curHP, param0->battleMons[param1].maxHP, (8 * 6));

    if ((param0->battleMons[param1].status & 0xff) || ((v1 != 4) && (v1 != 3))) {
        if (v2 == 1) {
            v0 = 11;
        } else {
            v0 = 11;
        }
    }

    return v0;
}

BOOL BattleSystem_CanPickCommand (BattleContext *battleSys, int battler)
{
    BOOL v0 = 1;

    if ((battleSys->battleMons[battler].statusVolatile & 0x400000)
            || (battleSys->battleMons[battler].statusVolatile & 0xc00)
            || (battleSys->battleMons[battler].statusVolatile & 0x70)
            || (battleSys->battleMons[battler].statusVolatile & 0x1000)) {
        v0 = 0;
    }

    return v0;
}

void ov16_02259A5C (BattleSystem * param0, BattleContext * param1, Pokemon * param2)
{
    TrainerInfo * v0;
    int v1;
    int v2;
    int v3;

    v0 = BattleSystem_TrainerInfo(param0, 0);
    v1 = BattleSystem_MapHeader(param0);
    v2 = BattleSystem_Terrain(param0);

    if (BattleSystem_BattleType(param0) & 0x200) {
        v3 = Pokemon_GetValue(param2, MON_DATA_POKEBALL, NULL);
    } else {
        v3 = param1->msgItemTemp;
    }

    sub_02077E64(param2, v0, v3, v1, v2, 5);
}

u8 BattleContext_IOBufferVal (BattleContext *battleCtx, int battler)
{
    return battleCtx->ioBuffer[battler][0];
}

BOOL Battler_SubstituteWasHit(BattleContext *battleCtx, int battler)
{
    BOOL result = FALSE;

    if (battleCtx->selfTurnFlags[battler].statusFlags & SELF_TURN_FLAG_SUBSTITUTE_HIT) {
        result = TRUE;
    }

    return result;
}

BOOL BattleSystem_TrainerIsOT (BattleSystem * param0, BattleContext * param1)
{
    TrainerInfo * v0;
    u32 v1;
    u32 v2;
    const u16 * v3;

    v0 = BattleSystem_TrainerInfo(param0, 0);
    v1 = TrainerInfo_ID(v0);
    v2 = TrainerInfo_Gender(v0);
    v3 = TrainerInfo_Name(v0);

    if ((v1 == param1->battleMons[param1->attacker].OTId) && (v2 == param1->battleMons[param1->attacker].OTGender) && (GF_strncmp(v3, &param1->battleMons[param1->attacker].OTName[0], 7) == 0)) {
        return 1;
    }

    return 0;
}

BOOL BattleSystem_PokemonIsOT (BattleSystem * param0, Pokemon * param1)
{
    TrainerInfo * v0;
    u32 v1;
    u32 v2;
    const u16 * v3;
    u16 v4[7 + 1];

    v0 = BattleSystem_TrainerInfo(param0, 0);
    v1 = TrainerInfo_ID(v0);
    v2 = TrainerInfo_Gender(v0);
    v3 = TrainerInfo_Name(v0);

    Pokemon_GetValue(param1, MON_DATA_OTNAME, &v4[0]);

    if ((v1 == Pokemon_GetValue(param1, MON_DATA_OT_ID, NULL)) && (v2 == Pokemon_GetValue(param1, MON_DATA_OT_GENDER, NULL)) && (GF_strncmp(v3, &v4[0], 7) == 0)) {
        return 1;
    }

    return 0;
}

BOOL BattleSystem_TriggerFormChange(BattleSystem *battleSys, BattleContext *battleCtx, int *subscript)
{
    int i;
    int arceusForm;
    BOOL result = FALSE;

    for (i = 0; i < BattleSystem_MaxBattlers(battleSys); i++) {
        battleCtx->msgBattlerTemp = battleCtx->monSpeedOrder[i];

        if (battleCtx->battleMons[battleCtx->msgBattlerTemp].species == SPECIES_CASTFORM
                && battleCtx->battleMons[battleCtx->msgBattlerTemp].curHP
                && Battler_Ability(battleCtx, battleCtx->msgBattlerTemp) == ABILITY_FORECAST) {
            if (NO_CLOUD_NINE) {
                if ((battleCtx->fieldConditionsMask & FIELD_CONDITION_CASTFORM) == FALSE
                        && battleCtx->battleMons[battleCtx->msgBattlerTemp].type1 != TYPE_NORMAL
                        && battleCtx->battleMons[battleCtx->msgBattlerTemp].type2 != TYPE_NORMAL) {
                    battleCtx->battleMons[battleCtx->msgBattlerTemp].type1 = TYPE_NORMAL;
                    battleCtx->battleMons[battleCtx->msgBattlerTemp].type2 = TYPE_NORMAL;
                    battleCtx->battleMons[battleCtx->msgBattlerTemp].formNum = 0;
                    *subscript = BATTLE_SUBSEQ_FORM_CHANGE;
                    result = TRUE;
                    break;
                } else if (WEATHER_IS_SUN
                        && battleCtx->battleMons[battleCtx->msgBattlerTemp].type1 != TYPE_FIRE
                        && battleCtx->battleMons[battleCtx->msgBattlerTemp].type2 != TYPE_FIRE) {
                    battleCtx->battleMons[battleCtx->msgBattlerTemp].type1 = TYPE_FIRE;
                    battleCtx->battleMons[battleCtx->msgBattlerTemp].type2 = TYPE_FIRE;
                    battleCtx->battleMons[battleCtx->msgBattlerTemp].formNum = 1;
                    *subscript = BATTLE_SUBSEQ_FORM_CHANGE;
                    result = TRUE;
                    break;
                } else if (WEATHER_IS_RAIN
                        && battleCtx->battleMons[battleCtx->msgBattlerTemp].type1 != TYPE_WATER
                        && battleCtx->battleMons[battleCtx->msgBattlerTemp].type2 != TYPE_WATER) {
                    battleCtx->battleMons[battleCtx->msgBattlerTemp].type1 = TYPE_WATER;
                    battleCtx->battleMons[battleCtx->msgBattlerTemp].type2 = TYPE_WATER;
                    battleCtx->battleMons[battleCtx->msgBattlerTemp].formNum = 2;
                    *subscript = BATTLE_SUBSEQ_FORM_CHANGE;
                    result = TRUE;
                    break;
                } else if (WEATHER_IS_HAIL
                        && battleCtx->battleMons[battleCtx->msgBattlerTemp].type1 != TYPE_ICE
                        && battleCtx->battleMons[battleCtx->msgBattlerTemp].type2 != TYPE_ICE) {
                    battleCtx->battleMons[battleCtx->msgBattlerTemp].type1 = TYPE_ICE;
                    battleCtx->battleMons[battleCtx->msgBattlerTemp].type2 = TYPE_ICE;
                    battleCtx->battleMons[battleCtx->msgBattlerTemp].formNum = 3;
                    *subscript = BATTLE_SUBSEQ_FORM_CHANGE;
                    result = TRUE;
                    break;
                }
            } else if (battleCtx->battleMons[battleCtx->msgBattlerTemp].type1 != TYPE_NORMAL
                    && battleCtx->battleMons[battleCtx->msgBattlerTemp].type2 != TYPE_NORMAL) {
                battleCtx->battleMons[battleCtx->msgBattlerTemp].type1 = TYPE_NORMAL;
                battleCtx->battleMons[battleCtx->msgBattlerTemp].type2 = TYPE_NORMAL;
                battleCtx->battleMons[battleCtx->msgBattlerTemp].formNum = 0;
                *subscript = BATTLE_SUBSEQ_FORM_CHANGE;
                result = TRUE;
                break;
            }
        }

        if (battleCtx->battleMons[battleCtx->msgBattlerTemp].species == SPECIES_CHERRIM
                && battleCtx->battleMons[battleCtx->msgBattlerTemp].curHP) {
            if (NO_CLOUD_NINE) {
                if ((battleCtx->fieldConditionsMask & FIELD_CONDITION_CASTFORM) == FALSE
                        && battleCtx->battleMons[battleCtx->msgBattlerTemp].formNum == 1) {
                    battleCtx->battleMons[battleCtx->msgBattlerTemp].formNum = 0;
                    *subscript = BATTLE_SUBSEQ_FORM_CHANGE;
                    result = TRUE;
                    break;
                } else if (WEATHER_IS_SUN && battleCtx->battleMons[battleCtx->msgBattlerTemp].formNum == 0) {
                    battleCtx->battleMons[battleCtx->msgBattlerTemp].formNum = 1;
                    *subscript = BATTLE_SUBSEQ_FORM_CHANGE;
                    result = TRUE;
                    break;
                } else if (WEATHER_IS_RAIN && battleCtx->battleMons[battleCtx->msgBattlerTemp].formNum == 1) {
                    battleCtx->battleMons[battleCtx->msgBattlerTemp].formNum = 0;
                    *subscript = BATTLE_SUBSEQ_FORM_CHANGE;
                    result = TRUE;
                    break;
                } else if (WEATHER_IS_HAIL && battleCtx->battleMons[battleCtx->msgBattlerTemp].formNum == 1) {
                    battleCtx->battleMons[battleCtx->msgBattlerTemp].formNum = 0;
                    *subscript = BATTLE_SUBSEQ_FORM_CHANGE;
                    result = TRUE;
                    break;
                }
            } else if (battleCtx->battleMons[battleCtx->msgBattlerTemp].formNum == 1) {
                battleCtx->battleMons[battleCtx->msgBattlerTemp].formNum = 0;
                *subscript = BATTLE_SUBSEQ_FORM_CHANGE;
                result = TRUE;
                break;
            }
        }

        if (battleCtx->battleMons[battleCtx->msgBattlerTemp].species == SPECIES_ARCEUS
                && battleCtx->battleMons[battleCtx->msgBattlerTemp].curHP
                && Battler_Ability(battleCtx, battleCtx->msgBattlerTemp) == ABILITY_MULTITYPE) {
            arceusForm = Pokemon_GetArceusTypeOf(Item_LoadParam(battleCtx->battleMons[battleCtx->msgBattlerTemp].heldItem, ITEM_PARAM_HOLD_EFFECT, HEAP_ID_BATTLE));

            if (battleCtx->battleMons[battleCtx->msgBattlerTemp].formNum != arceusForm) {
                battleCtx->battleMons[battleCtx->msgBattlerTemp].formNum = arceusForm;
                *subscript = BATTLE_SUBSEQ_FORM_CHANGE;
                result = TRUE;
                break;
            }
        }

        if (battleCtx->battleMons[battleCtx->msgBattlerTemp].species == SPECIES_GIRATINA
                && battleCtx->battleMons[battleCtx->msgBattlerTemp].curHP
                && battleCtx->battleMons[battleCtx->msgBattlerTemp].formNum == 1
                && ((battleCtx->battleMons[battleCtx->msgBattlerTemp].statusVolatile & VOLATILE_CONDITION_TRANSFORM)
                    || ((BattleSystem_BattleStatus(battleSys) & BATTLE_STATUS_DISTORTION) == FALSE
                        && battleCtx->battleMons[battleCtx->msgBattlerTemp].heldItem != ITEM_GRISEOUS_ORB))) {
            if (battleCtx->battleMons[battleCtx->msgBattlerTemp].statusVolatile & VOLATILE_CONDITION_TRANSFORM) {
                Pokemon *mon = Pokemon_New(HEAP_ID_BATTLE);

                int target;
                if (BattleSystem_BattleType(battleSys) & BATTLE_TYPE_DOUBLES) {
                    target = battleCtx->battlerActions[battleCtx->msgBattlerTemp][BATTLE_ACTION_CHOOSE_TARGET];
                } else {
                    target = battleCtx->msgBattlerTemp ^ 1;
                }

                Pokemon_Copy(BattleSystem_PartyPokemon(battleSys, target, battleCtx->selectedPartySlot[target]), mon);
                
                // Don't copy the Griseous Orb
                int tmp = ITEM_NONE;
                Pokemon_SetValue(mon, MON_DATA_HELD_ITEM, &tmp);
                
                // Force Giratina-Altered form
                tmp = 0;
                Pokemon_SetValue(mon, MON_DATA_FORM, &tmp);
                Pokemon_SetGiratinaForm(mon);

                battleCtx->battleMons[battleCtx->msgBattlerTemp].attack = Pokemon_GetValue(mon, MON_DATA_ATK, 0);
                battleCtx->battleMons[battleCtx->msgBattlerTemp].defense = Pokemon_GetValue(mon, MON_DATA_DEF, 0);
                battleCtx->battleMons[battleCtx->msgBattlerTemp].speed = Pokemon_GetValue(mon, MON_DATA_SPEED, 0);
                battleCtx->battleMons[battleCtx->msgBattlerTemp].spAttack = Pokemon_GetValue(mon, MON_DATA_SP_ATK, 0);
                battleCtx->battleMons[battleCtx->msgBattlerTemp].spDefense = Pokemon_GetValue(mon, MON_DATA_SP_DEF, 0);
                battleCtx->battleMons[battleCtx->msgBattlerTemp].ability = Pokemon_GetValue(mon, MON_DATA_ABILITY, 0);
                battleCtx->battleMons[battleCtx->msgBattlerTemp].formNum = 0;
                battleCtx->battleStatusMask2 |= SYSCTL_FORM_CHANGE;

                BattleIO_UpdatePartyMon(battleSys, battleCtx, battleCtx->msgBattlerTemp);
                Heap_FreeToHeap(mon);

                *subscript = BATTLE_SUBSEQ_FORM_CHANGE;
                result = TRUE;
                break;
            } else {
                *subscript = BATTLE_SUBSEQ_GIRATINA_FORM_CHANGE;
                result = TRUE;
                break;
            }
        }
    }

    return result;
}

void ov16_0225A1B0 (BattleSystem * param0, BattleContext * param1)
{
    int v0;
    int v1;

    for (v0 = 0; v0 < BattleSystem_MaxBattlers(param0); v0++) {
        for (v1 = 0; v1 < 6; v1++) {
            param1->partyOrder[v0][v1] = v1;
        }

        BattleSystem_SwitchSlots(param0, param1, v0, param1->selectedPartySlot[v0]);
    }
}

void BattleSystem_SwitchSlots(BattleSystem *battleSys, BattleContext *battleCtx, int battler, int partySlot)
{
    // declare C89-style to match
    int i;
    int tmp;
    int targetSlot;
    u32 battleType = BattleSystem_BattleType(battleSys);

    if (((battleType & BATTLE_TYPE_DOUBLES) && (battleType & BATTLE_TYPE_2vs2_TAG) == FALSE)
            || ((battleType & BATTLE_TYPE_TAG) && (BattleSystem_BattlerSlot(battleSys, battler) & BATTLER_TYPE_SOLO_ENEMY) == FALSE)) {
        if (BattleSystem_BattlerSlot(battleSys, battler) == BATTLER_TYPE_PLAYER_SIDE_SLOT_2
                || BattleSystem_BattlerSlot(battleSys, battler) == BATTLER_TYPE_ENEMY_SIDE_SLOT_2) {
            targetSlot = 1;
        } else {
            targetSlot = 0;
        }

        battler &= 1;
    } else {
        targetSlot = 0;
    }

    for (i = 0; i < MAX_PARTY_SIZE; i++) {
        if (battleCtx->partyOrder[battler][i] == partySlot) {
            break;
        }
    }

    tmp = battleCtx->partyOrder[battler][targetSlot];
    battleCtx->partyOrder[battler][targetSlot] = battleCtx->partyOrder[battler][i];
    battleCtx->partyOrder[battler][i] = tmp;
}

typedef struct ItemEffectTypePair {
    u8 itemEffect;
    u8 type;
} ItemEffectTypePair;

static const ItemEffectTypePair sTypeBoostingItems[] = {
    { HOLD_EFFECT_STRENGTHEN_BUG,      TYPE_BUG      },
    { HOLD_EFFECT_STRENGTHEN_STEEL,    TYPE_STEEL    },
    { HOLD_EFFECT_STRENGTHEN_GROUND,   TYPE_GROUND   },
    { HOLD_EFFECT_STRENGTHEN_ROCK,     TYPE_ROCK     },
    { HOLD_EFFECT_STRENGTHEN_GRASS,    TYPE_GRASS    },
    { HOLD_EFFECT_STRENGTHEN_DARK,     TYPE_DARK     },
    { HOLD_EFFECT_STRENGTHEN_FIGHT,    TYPE_FIGHTING },
    { HOLD_EFFECT_STRENGTHEN_ELECTRIC, TYPE_ELECTRIC },
    { HOLD_EFFECT_STRENGTHEN_WATER,    TYPE_WATER    },
    { HOLD_EFFECT_STRENGTHEN_FLYING,   TYPE_FLYING   },
    { HOLD_EFFECT_STRENGTHEN_POISON,   TYPE_POISON   },
    { HOLD_EFFECT_STRENGTHEN_ICE,      TYPE_ICE      },
    { HOLD_EFFECT_STRENGTHEN_GHOST,    TYPE_GHOST    },
    { HOLD_EFFECT_STRENGTHEN_PSYCHIC,  TYPE_PSYCHIC  },
    { HOLD_EFFECT_STRENGTHEN_FIRE,     TYPE_FIRE     },
    { HOLD_EFFECT_STRENGTHEN_DRAGON,   TYPE_DRAGON   },
    { HOLD_EFFECT_STRENGTHEN_NORMAL,   TYPE_NORMAL   },
    { HOLD_EFFECT_ARCEUS_FIRE,         TYPE_FIRE     },
    { HOLD_EFFECT_ARCEUS_WATER,        TYPE_WATER    },
    { HOLD_EFFECT_ARCEUS_ELECTRIC,     TYPE_ELECTRIC },
    { HOLD_EFFECT_ARCEUS_GRASS,        TYPE_GRASS    },
    { HOLD_EFFECT_ARCEUS_ICE,          TYPE_ICE      },
    { HOLD_EFFECT_ARCEUS_FIGHT,        TYPE_FIGHTING },
    { HOLD_EFFECT_ARCEUS_POISON,       TYPE_POISON   },
    { HOLD_EFFECT_ARCEUS_GROUND,       TYPE_GROUND   },
    { HOLD_EFFECT_ARCEUS_FLYING,       TYPE_FLYING   },
    { HOLD_EFFECT_ARCEUS_PSYCHIC,      TYPE_PSYCHIC  },
    { HOLD_EFFECT_ARCEUS_BUG,          TYPE_BUG      },
    { HOLD_EFFECT_ARCEUS_ROCK,         TYPE_ROCK     },
    { HOLD_EFFECT_ARCEUS_GHOST,        TYPE_GHOST    },
    { HOLD_EFFECT_ARCEUS_DRAGON,       TYPE_DRAGON   },
    { HOLD_EFFECT_ARCEUS_DARK,         TYPE_DARK     },
    { HOLD_EFFECT_ARCEUS_STEEL,        TYPE_STEEL    }
};

static const Fraction sStatStageBoosts[] = {
    { 10, 40 }, // -6
    { 10, 35 }, // -5
    { 10, 30 }, // -4
    { 10, 25 }, // -3
    { 10, 20 }, // -2
    { 10, 15 }, // -1
    { 10, 10 }, // neutral
    { 15, 10 }, // +1
    { 20, 10 }, // +2
    { 25, 10 }, // +3
    { 30, 10 }, // +4
    { 35, 10 }, // +5
    { 40, 10 }, // +6
};

static const u16 sPunchingMoves[] = {
    MOVE_ICE_PUNCH,
    MOVE_FIRE_PUNCH,
    MOVE_THUNDER_PUNCH,
    MOVE_MACH_PUNCH,
    MOVE_FOCUS_PUNCH,
    MOVE_DIZZY_PUNCH,
    MOVE_DYNAMIC_PUNCH,
    MOVE_HAMMER_ARM,
    MOVE_MEGA_PUNCH,
    MOVE_COMET_PUNCH,
    MOVE_METEOR_MASH,
    MOVE_SHADOW_PUNCH,
    MOVE_DRAIN_PUNCH,
    MOVE_BULLET_PUNCH,
    MOVE_SKY_UPPERCUT
};

typedef struct DamageCalcParams {
    u16 species;
    s16 curHP;
    u16 maxHP;
    u16 _padding06;

    int heldItemEffect;
    int heldItemPower;

    u32 statusMask;

    u8 ability;
    u8 gender;
    u8 type1;
    u8 type2;
} DamageCalcParams;

int BattleSystem_CalcMoveDamage(BattleSystem *battleSys,
    BattleContext *battleCtx,
    int move,
    u32 sideConditions,
    u32 fieldConditions,
    u16 inPower,
    u8 inType,
    u8 attacker,
    u8 defender,
    u8 criticalMul)
{
    // vars have to all be declared C89-style to match
    int i;
    s32 damage = 0;
    s32 stageDivisor = 0;
    u8 moveType;
    u8 moveClass;
    u16 attackStat;
    u16 defenseStat;
    u16 spAttackStat;
    u16 spDefenseStat;
    s8 attackStage;
    s8 defenseStage;
    s8 spAttackStage;
    s8 spDefenseStage;
    u8 attackerLevel;
    u16 movePower;
    u16 itemTmp;
    u32 battleType;
    DamageCalcParams attackerParams;
    DamageCalcParams defenderParams;

    GF_ASSERT(criticalMul == 1 || criticalMul > 1);

    attackStat = BattleMon_Get(battleCtx, attacker, BATTLEMON_ATTACK, NULL);
    defenseStat = BattleMon_Get(battleCtx, defender, BATTLEMON_DEFENSE, NULL);
    spAttackStat = BattleMon_Get(battleCtx, attacker, BATTLEMON_SP_ATTACK, NULL);
    spDefenseStat = BattleMon_Get(battleCtx, defender, BATTLEMON_SP_DEFENSE, NULL);
    attackStage = BattleMon_Get(battleCtx, attacker, BATTLEMON_ATTACK_STAGE, NULL) - 6;
    defenseStage = BattleMon_Get(battleCtx, defender, BATTLEMON_DEFENSE_STAGE, NULL) - 6;
    spAttackStage = BattleMon_Get(battleCtx, attacker, BATTLEMON_SP_ATTACK_STAGE, NULL) - 6;
    spDefenseStage = BattleMon_Get(battleCtx, defender, BATTLEMON_SP_DEFENSE_STAGE, NULL) - 6;
    attackerLevel = BattleMon_Get(battleCtx, attacker, BATTLEMON_LEVEL, NULL);

    attackerParams.species = BattleMon_Get(battleCtx, attacker, BATTLEMON_SPECIES, NULL);
    defenderParams.species = BattleMon_Get(battleCtx, defender, BATTLEMON_SPECIES, NULL);
    attackerParams.curHP = BattleMon_Get(battleCtx, attacker, BATTLEMON_CUR_HP, NULL);
    defenderParams.curHP = BattleMon_Get(battleCtx, defender, BATTLEMON_CUR_HP, NULL);
    attackerParams.maxHP = BattleMon_Get(battleCtx, attacker, BATTLEMON_MAX_HP, NULL);
    defenderParams.maxHP = BattleMon_Get(battleCtx, defender, BATTLEMON_MAX_HP, NULL);
    attackerParams.statusMask = BattleMon_Get(battleCtx, attacker, BATTLEMON_STATUS, NULL);
    defenderParams.statusMask = BattleMon_Get(battleCtx, defender, BATTLEMON_STATUS, NULL);
    attackerParams.ability = Battler_Ability(battleCtx, attacker);
    defenderParams.ability = Battler_Ability(battleCtx, defender);
    attackerParams.gender = BattleMon_Get(battleCtx, attacker, BATTLEMON_GENDER, NULL);
    defenderParams.gender = BattleMon_Get(battleCtx, defender, BATTLEMON_GENDER, NULL);
    attackerParams.type1 = BattleMon_Get(battleCtx, attacker, BATTLEMON_TYPE_1, NULL);
    defenderParams.type1 = BattleMon_Get(battleCtx, defender, BATTLEMON_TYPE_1, NULL);
    attackerParams.type2 = BattleMon_Get(battleCtx, attacker, BATTLEMON_TYPE_2, NULL);
    defenderParams.type2 = BattleMon_Get(battleCtx, defender, BATTLEMON_TYPE_2, NULL);

    itemTmp = Battler_HeldItem(battleCtx, attacker);
    attackerParams.heldItemEffect = BattleSystem_GetItemData(battleCtx, itemTmp, ITEM_PARAM_HOLD_EFFECT);
    attackerParams.heldItemPower = BattleSystem_GetItemData(battleCtx, itemTmp, ITEM_PARAM_HOLD_EFFECT_PARAM);

    itemTmp = Battler_HeldItem(battleCtx, defender);
    defenderParams.heldItemEffect = BattleSystem_GetItemData(battleCtx, itemTmp, ITEM_PARAM_HOLD_EFFECT);
    defenderParams.heldItemPower = BattleSystem_GetItemData(battleCtx, itemTmp, ITEM_PARAM_HOLD_EFFECT_PARAM);

    battleType = BattleSystem_BattleType(battleSys);

    // Assign power; prefer the input power (used by variable-power moves, e.g. Gyro Ball)
    if (inPower == 0) {
        movePower = MOVE_DATA(move).power;
    } else {
        movePower = inPower;
    }

    if (attackerParams.ability == ABILITY_NORMALIZE) {
        moveType = TYPE_NORMAL;
    } else if (inType == TYPE_NORMAL) {
        moveType = MOVE_DATA(move).type;
    } else {
        moveType = inType & 0x3F;
    }

    GF_ASSERT(battleCtx->powerMul >= 10);
    movePower = movePower * battleCtx->powerMul / 10;

    if ((battleCtx->battleMons[attacker].moveEffectsMask & MOVE_EFFECT_CHARGE) && moveType == TYPE_ELECTRIC) {
        movePower *= 2;
    }

    if (battleCtx->turnFlags[attacker].helpingHand) {
        movePower = movePower * 15 / 10;
    }

    if (attackerParams.ability == ABILITY_TECHNICIAN
            && move != MOVE_STRUGGLE
            && movePower <= 60) {
        movePower = movePower * 15 / 10;
    }

    moveClass = MOVE_DATA(move).class;

    if (attackerParams.ability == ABILITY_HUGE_POWER || attackerParams.ability == ABILITY_PURE_POWER) {
        attackStat = attackStat * 2;
    }
    if (attackerParams.ability == ABILITY_SLOW_START
            && BattleContext_Get(battleSys, battleCtx, BATTLECTX_TOTAL_TURNS, NULL)
                - BattleMon_Get(battleCtx, attacker, BATTLEMON_SLOW_START_TURN_NUMBER, NULL)
                < 5) {
        attackStat /= 2;
    }

    for (i = 0; i < NELEMS(sTypeBoostingItems); i++) {
        if (attackerParams.heldItemEffect == sTypeBoostingItems[i].itemEffect
                && moveType == sTypeBoostingItems[i].type) {
            movePower = movePower * (100 + attackerParams.heldItemPower) / 100;
            break;
        }
    }

    if (attackerParams.heldItemEffect == HOLD_EFFECT_CHOICE_ATK) {
        attackStat = attackStat * 150 / 100;
    }
    if (attackerParams.heldItemEffect == HOLD_EFFECT_CHOICE_SPATK) {
        spAttackStat = spAttackStat * 150 / 100;
    }
    if (attackerParams.heldItemEffect == HOLD_EFFECT_LATI_SPECIAL
            && (battleType & BATTLE_TYPE_FRONTIER) == FALSE
            && (attackerParams.species == SPECIES_LATIOS || attackerParams.species == SPECIES_LATIAS)) {
        spAttackStat = spAttackStat * 150 / 100;
    }
    if (defenderParams.heldItemEffect == HOLD_EFFECT_LATI_SPECIAL
            && (battleType & BATTLE_TYPE_FRONTIER) == FALSE
            && (defenderParams.species == SPECIES_LATIOS || defenderParams.species == SPECIES_LATIAS)) {
        spDefenseStat = spDefenseStat * 150 / 100;
    }
    if (attackerParams.heldItemEffect == HOLD_EFFECT_CLAMPERL_SPATK
            && attackerParams.species == SPECIES_CLAMPERL) {
        spAttackStat *= 2;
    }
    if (defenderParams.heldItemEffect == HOLD_EFFECT_CLAMPERL_SPDEF
            && defenderParams.species == SPECIES_CLAMPERL) {
        spDefenseStat *= 2;
    }
    if (attackerParams.heldItemEffect == HOLD_EFFECT_PIKA_SPATK_UP
            && attackerParams.species == SPECIES_PIKACHU) {
        movePower *= 2;
    }
    if (defenderParams.heldItemEffect == HOLD_EFFECT_DITTO_DEF_UP
            && defenderParams.species == SPECIES_DITTO) {
        defenseStat *= 2;
    }
    if (attackerParams.heldItemEffect == HOLD_EFFECT_CUBONE_ATK_UP
            && (attackerParams.species == SPECIES_CUBONE || attackerParams.species == SPECIES_MAROWAK)) {
        attackStat *= 2;
    }
    if (attackerParams.heldItemEffect == HOLD_EFFECT_DIALGA_BOOST
            && (moveType == TYPE_DRAGON || moveType == TYPE_STEEL)
            && attackerParams.species == SPECIES_DIALGA) {
        movePower = movePower * (100 + attackerParams.heldItemPower) / 100;
    }
    if (attackerParams.heldItemEffect == HOLD_EFFECT_PALKIA_BOOST
            && (moveType == TYPE_DRAGON || moveType == TYPE_WATER)
            && attackerParams.species == SPECIES_PALKIA) {
        movePower = movePower * (100 + attackerParams.heldItemPower) / 100;
    }
    if (attackerParams.heldItemEffect == HOLD_EFFECT_GIRATINA_BOOST
            && (moveType == TYPE_DRAGON || moveType == TYPE_GHOST)
            && (BattleMon_Get(battleCtx, attacker, BATTLEMON_VOLATILE_STATUS, NULL) & VOLATILE_CONDITION_TRANSFORM) == FALSE
            && attackerParams.species == SPECIES_GIRATINA) {
        movePower = movePower * (100 + attackerParams.heldItemPower) / 100;
    }
    if (attackerParams.heldItemEffect == HOLD_EFFECT_POWER_UP_PHYS
            && moveClass == CLASS_PHYSICAL) {
        movePower = movePower * (100 + attackerParams.heldItemPower) / 100;
    }
    if (attackerParams.heldItemEffect == HOLD_EFFECT_POWER_UP_SPEC
            && moveClass == CLASS_SPECIAL) {
        movePower = movePower * (100 + attackerParams.heldItemPower) / 100;
    }

    if (Battler_IgnorableAbility(battleCtx, attacker, defender, ABILITY_THICK_FAT) == TRUE
            && (moveType == TYPE_FIRE || moveType == TYPE_ICE)) {
        movePower /= 2;
    }

    if (attackerParams.ability == ABILITY_HUSTLE) {
        attackStat = attackStat * 150 / 100;
    }
    if (attackerParams.ability == ABILITY_GUTS && attackerParams.statusMask) {
        attackStat = attackStat * 150 / 100;
    }

    if (Battler_IgnorableAbility(battleCtx, attacker, defender, ABILITY_MARVEL_SCALE) == TRUE
            && defenderParams.statusMask) {
        defenseStat = defenseStat * 150 / 100;
    }

    if (attackerParams.ability == ABILITY_PLUS
            && BattleSystem_CountAbility(battleSys, battleCtx, COUNT_ALIVE_BATTLERS_OUR_SIDE, attacker, ABILITY_MINUS)) {
        spAttackStat = spAttackStat * 150 / 100;
    }
    if (attackerParams.ability == ABILITY_MINUS
            && BattleSystem_CountAbility(battleSys, battleCtx, COUNT_ALIVE_BATTLERS_OUR_SIDE, attacker, ABILITY_PLUS)) {
        spAttackStat = spAttackStat * 150 / 100;
    }

    if (moveType == TYPE_ELECTRIC
            && BattleSystem_AnyBattlersWithMoveEffect(battleSys, battleCtx, MOVE_EFFECT_MUD_SPORT)) {
        movePower /= 2;
    }
    if (moveType == TYPE_FIRE
            && BattleSystem_AnyBattlersWithMoveEffect(battleSys, battleCtx, MOVE_EFFECT_WATER_SPORT)) {
        movePower /= 2;
    }

    if (moveType == TYPE_GRASS
            && attackerParams.ability == ABILITY_OVERGROW
            && attackerParams.curHP <= (attackerParams.maxHP / 3)) {
        movePower = movePower * 150 / 100;
    }
    if (moveType == TYPE_FIRE
            && attackerParams.ability == ABILITY_BLAZE
            && attackerParams.curHP <= (attackerParams.maxHP / 3)) {
        movePower = movePower * 150 / 100;
    }
    if (moveType == TYPE_WATER
            && attackerParams.ability == ABILITY_TORRENT
            && attackerParams.curHP <= (attackerParams.maxHP / 3)) {
        movePower = movePower * 150 / 100;
    }
    if (moveType == TYPE_BUG
            && attackerParams.ability == ABILITY_SWARM
            && attackerParams.curHP <= (attackerParams.maxHP / 3)) {
        movePower = movePower * 150 / 100;
    }

    if (moveType == TYPE_FIRE
            && Battler_IgnorableAbility(battleCtx, attacker, defender, ABILITY_HEATPROOF) == TRUE) {
        movePower /= 2;
    }
    if (moveType == TYPE_FIRE
            && Battler_IgnorableAbility(battleCtx, attacker, defender, ABILITY_DRY_SKIN) == TRUE) {
        movePower = movePower * 125 / 100;
    }

    if (attackerParams.ability == ABILITY_SIMPLE) {
        attackStage *= 2;
        if (attackStage < -6) {
            attackStage = -6;
        }
        if (attackStage > 6) {
            attackStage = 6;
        }

        spAttackStage *= 2;
        if (spAttackStage < -6) {
            spAttackStage = -6;
        }
        if (spAttackStage > 6) {
            spAttackStage = 6;
        }
    }

    if (Battler_IgnorableAbility(battleCtx, attacker, defender, ABILITY_SIMPLE) == TRUE) {
        defenseStage *= 2;
        if (defenseStage < -6) {
            defenseStage = -6;
        }
        if (defenseStage > 6) {
            defenseStage = 6;
        }

        spDefenseStage *= 2;
        if (spDefenseStage < -6) {
            spDefenseStage = -6;
        }
        if (spDefenseStage > 6) {
            spDefenseStage = 6;
        }
    }

    if (Battler_IgnorableAbility(battleCtx, attacker, defender, ABILITY_UNAWARE) == TRUE) {
        attackStage = 0;
        spAttackStage = 0;
    }

    if (attackerParams.ability == ABILITY_UNAWARE) {
        defenseStage = 0;
        spDefenseStage = 0;
    }

    attackStage += 6;
    defenseStage += 6;
    spAttackStage += 6;
    spDefenseStage += 6;

    if (attackerParams.ability == ABILITY_RIVALRY
            && attackerParams.gender == defenderParams.gender
            && attackerParams.gender != GENDER_NONE
            && defenderParams.gender != GENDER_NONE) {
        movePower = movePower * 125 / 100;
    }
    if (attackerParams.ability == ABILITY_RIVALRY
            && attackerParams.gender != defenderParams.gender
            && attackerParams.gender != GENDER_NONE
            && defenderParams.gender != GENDER_NONE) {
        movePower = movePower * 75 / 100;
    }

    for (i = 0; i < NELEMS(sPunchingMoves); i++) {
        if (sPunchingMoves[i] == move && attackerParams.ability == ABILITY_IRON_FIST) {
            movePower = movePower * 12 / 10;
            break;
        }
    }

    if (NO_CLOUD_NINE) {
        if ((fieldConditions & FIELD_CONDITION_SUNNY) && attackerParams.ability == ABILITY_SOLAR_POWER) {
            spAttackStat = spAttackStat * 15 / 10;
        }

        if ((fieldConditions & FIELD_CONDITION_SANDSTORM)
                && (defenderParams.type1 == TYPE_ROCK || defenderParams.type2 == TYPE_ROCK)) {
            spDefenseStat = spDefenseStat * 15 / 10;
        }

        if ((fieldConditions & FIELD_CONDITION_SUNNY)
                && BattleSystem_CountAbility(battleSys, battleCtx, COUNT_ALIVE_BATTLERS_OUR_SIDE, attacker, ABILITY_FLOWER_GIFT)) {
            attackStat = attackStat * 15 / 10;
        }

        if ((fieldConditions & FIELD_CONDITION_SUNNY)
                && Battler_Ability(battleCtx, attacker) != ABILITY_MOLD_BREAKER
                && BattleSystem_CountAbility(battleSys, battleCtx, COUNT_ALIVE_BATTLERS_OUR_SIDE, defender, ABILITY_FLOWER_GIFT)) {
            spDefenseStat = spDefenseStat * 15 / 10;
        }
    }

    if (MOVE_DATA(move).effect == BATTLE_EFFECT_HALVE_DEFENSE) {
        defenseStat = defenseStat / 2;
    }

    if (moveClass == CLASS_PHYSICAL) {
        if (criticalMul > 1) {
            if (attackStage > 6) {
                damage = attackStat * sStatStageBoosts[attackStage].numerator;
                damage /= sStatStageBoosts[attackStage].denominator;
            } else {
                damage = attackStat;
            }
        } else {
            damage = attackStat * sStatStageBoosts[attackStage].numerator;
            damage /= sStatStageBoosts[attackStage].denominator;
        }

        damage *= movePower;
        damage *= (attackerLevel * 2 / 5 + 2);

        if (criticalMul > 1) {
            if (defenseStage < 6) {
                stageDivisor = defenseStat * sStatStageBoosts[defenseStage].numerator;
                stageDivisor /= sStatStageBoosts[defenseStage].denominator;
            } else {
                stageDivisor = defenseStat;
            }
        } else {
            stageDivisor = defenseStat * sStatStageBoosts[defenseStage].numerator;
            stageDivisor /= sStatStageBoosts[defenseStage].denominator;
        }

        damage /= stageDivisor;
        damage /= 50;

        if ((attackerParams.statusMask & MON_CONDITION_BURN) && attackerParams.ability != ABILITY_GUTS) {
            damage /= 2;
        }

        if ((sideConditions & SIDE_CONDITION_REFLECT) != FALSE
                && criticalMul == 1
                && MOVE_DATA(move).effect != BATTLE_EFFECT_REMOVE_SCREENS) {
            if ((battleType & BATTLE_TYPE_DOUBLES)
                    && BattleSystem_CountAliveBattlers(battleSys, battleCtx, TRUE, defender) == 2) {
                damage = damage * 2 / 3;
            } else {
                damage /= 2;
            }
        }
    } else if (moveClass == CLASS_SPECIAL) {
        if (criticalMul > 1) {
            if (spAttackStage > 6) {
                damage = spAttackStat * sStatStageBoosts[spAttackStage].numerator;
                damage /= sStatStageBoosts[spAttackStage].denominator;
            } else {
                damage = spAttackStat;
            }
        } else {
            damage = spAttackStat * sStatStageBoosts[spAttackStage].numerator;
            damage /= sStatStageBoosts[spAttackStage].denominator;
        }

        damage *= movePower;
        damage *= (attackerLevel * 2 / 5 + 2);

        if (criticalMul > 1) {
            if (spDefenseStage < 6) {
                stageDivisor = spDefenseStat * sStatStageBoosts[spDefenseStage].numerator;
                stageDivisor /= sStatStageBoosts[spDefenseStage].denominator;
            } else {
                stageDivisor = spDefenseStat;
            }
        } else {
            stageDivisor = spDefenseStat * sStatStageBoosts[spDefenseStage].numerator;
            stageDivisor /= sStatStageBoosts[spDefenseStage].denominator;
        }

        damage /= stageDivisor;
        damage /= 50;

        if ((sideConditions & SIDE_CONDITION_LIGHT_SCREEN) != FALSE
                && criticalMul == 1
                && MOVE_DATA(move).effect != BATTLE_EFFECT_REMOVE_SCREENS) {
            if ((battleType & BATTLE_TYPE_DOUBLES)
                    && BattleSystem_CountAliveBattlers(battleSys, battleCtx, TRUE, defender) == 2) {
                damage = damage * 2 / 3;
            } else {
                damage /= 2;
            }
        }
    }

    if ((battleType & BATTLE_TYPE_DOUBLES)
            && MOVE_DATA(move).range == RANGE_ADJACENT_OPPONENTS
            && BattleSystem_CountAliveBattlers(battleSys, battleCtx, TRUE, defender) == 2) {
        damage = damage * 3 / 4;
    }
    if ((battleType & BATTLE_TYPE_DOUBLES)
            && MOVE_DATA(move).range == RANGE_ALL_ADJACENT
            && BattleSystem_CountAliveBattlers(battleSys, battleCtx, FALSE, defender) >= 2) {
        damage = damage * 3 / 4;
    }

    if (NO_CLOUD_NINE) {
        if (fieldConditions & FIELD_CONDITION_RAINING) {
            switch (moveType) {
            case TYPE_FIRE:
                damage /= 2;
                break;
            case TYPE_WATER:
                damage = damage * 15 / 10;
                break;
            }
        }

        if ((fieldConditions & FIELD_CONDITION_SOLAR_DOWN) && move == MOVE_SOLAR_BEAM) {
            damage /= 2;
        }

        if (fieldConditions & FIELD_CONDITION_SUNNY) {
            switch (moveType) {
            case TYPE_FIRE:
                damage = damage * 15 / 10;
                break;
            case TYPE_WATER:
                damage /= 2;
                break;
            }
        }
    }

    if (BattleMon_Get(battleCtx, attacker, BATTLEMON_FLASH_FIRE, NULL) && moveType == TYPE_FIRE) {
        damage = damage * 15 / 10;
    }

    return damage + 2;
}

int BattleSystem_CalcDamageVariance(BattleSystem *battleSys, BattleContext *battleCtx, int damage)
{
    if (damage) {
        damage *= (100 - (BattleSystem_RandNext(battleSys) % 16));
        damage /= 100;

        // Always deal at least 1 damage
        if (damage == 0) {
            damage = 1;
        }
    }

    return damage;
}

// each value here is implicitly 1 / N
static const u8 sCriticalStageRates[] = {
    16, // neutral
     8, // +1
     4, // +2
     3, // +3
     2, // +4
};

int BattleSystem_CalcCriticalMulti(BattleSystem *battleSys, BattleContext *battleCtx, int attacker, int defender, int criticalStage, u32 sideConditions)
{
    // have to declare vars C89-style to match
    u16 effectiveCritStage;
    u16 item;
    int itemEffect;
    u16 attackerSpecies;
    u32 attackerVolStatus;
    u32 defenderMoveEffects;
    int criticalMul = 1;
    int attackerAbility;

    item = Battler_HeldItem(battleCtx, attacker);
    itemEffect = BattleSystem_GetItemData(battleCtx, item, ITEM_PARAM_HOLD_EFFECT);
    attackerSpecies = battleCtx->battleMons[attacker].species;
    attackerVolStatus = battleCtx->battleMons[attacker].statusVolatile;
    defenderMoveEffects = battleCtx->battleMons[defender].moveEffectsMask;
    attackerAbility = battleCtx->battleMons[attacker].ability;
    effectiveCritStage = (((attackerVolStatus & VOLATILE_CONDITION_FOCUS_ENERGY) != FALSE) * 2)
            + (itemEffect == HOLD_EFFECT_CRITRATE_UP)
            + criticalStage
            + (attackerAbility == ABILITY_SUPER_LUCK)
            + (2 * (itemEffect == HOLD_EFFECT_CHANSEY_CRITRATE_UP && attackerSpecies == SPECIES_CHANSEY))
            + (2 * (itemEffect == HOLD_EFFECT_FARFETCHD_CRITRATE_UP && attackerSpecies == SPECIES_FARFETCHD));

    if (effectiveCritStage > 4) {
        effectiveCritStage = 4;
    }

    if (BattleSystem_RandNext(battleSys) % sCriticalStageRates[effectiveCritStage] == 0
            && Battler_IgnorableAbility(battleCtx, attacker, defender, ABILITY_BATTLE_ARMOR) == FALSE
            && Battler_IgnorableAbility(battleCtx, attacker, defender, ABILITY_SHELL_ARMOR) == FALSE
            && (sideConditions & SIDE_CONDITION_LUCKY_CHANT) == FALSE
            && (defenderMoveEffects & MOVE_EFFECT_NO_CRITICAL) == FALSE) {
        criticalMul = 2;
    }

    if (criticalMul == 2 && Battler_Ability(battleCtx, attacker) == ABILITY_SNIPER) {
        criticalMul = 3;
    }

    return criticalMul;
}

#define FORBIDDEN_BY_MIMIC_DELIM        0xFFFE
#define FORBIDDEN_BY_METRONOME_DELIM    0xFFFF

static const u16 sCannotMetronomeMoves[] = {
    MOVE_METRONOME,
    MOVE_STRUGGLE,
    MOVE_SKETCH,
    MOVE_MIMIC,
    MOVE_CHATTER,
    FORBIDDEN_BY_MIMIC_DELIM,
    MOVE_SLEEP_TALK,
    MOVE_ASSIST,
    MOVE_MIRROR_MOVE,
    MOVE_COUNTER,
    MOVE_MIRROR_COAT,
    MOVE_PROTECT,
    MOVE_DETECT,
    MOVE_ENDURE,
    MOVE_DESTINY_BOND,
    MOVE_THIEF,
    MOVE_FOLLOW_ME,
    MOVE_SNATCH,
    MOVE_HELPING_HAND,
    MOVE_COVET,
    MOVE_TRICK,
    MOVE_FOCUS_PUNCH,
    MOVE_FEINT,
    MOVE_COPYCAT,
    MOVE_ME_FIRST,
    MOVE_SWITCHEROO,
    FORBIDDEN_BY_METRONOME_DELIM,
};

BOOL Move_CanBeMimicked(u16 move)
{
    int i;
    for (i = 0; sCannotMetronomeMoves[i] != FORBIDDEN_BY_MIMIC_DELIM; i++) {
        if (sCannotMetronomeMoves[i] == move) {
            break;
        }
    }

    return sCannotMetronomeMoves[i] == FORBIDDEN_BY_MIMIC_DELIM;
}

BOOL Move_CanBeMetronomed(BattleSystem *battleSys, BattleContext *battleCtx, int battler, u16 move)
{
    int i = 0;

    if (BattleSystem_FailsInHighGravity(battleSys, battleCtx, battler, move) == TRUE
            || BattleSystem_HealBlocked(battleSys, battleCtx, battler, move) == TRUE) {
        return FALSE;
    }

    for (; sCannotMetronomeMoves[i] != FORBIDDEN_BY_METRONOME_DELIM; i++) {
        if (sCannotMetronomeMoves[i] == move) {
            break;
        }
    }

    return sCannotMetronomeMoves[i] == FORBIDDEN_BY_METRONOME_DELIM;
}

static const u16 sCannotEncoreMoves[] = {
    MOVE_TRANSFORM,
    MOVE_MIMIC,
    MOVE_SKETCH,
    MOVE_MIRROR_MOVE,
    MOVE_ENCORE,
    MOVE_STRUGGLE,
};

BOOL Move_CanBeEncored(BattleContext *battleCtx, u16 move)
{
    int i;
    for (i = 0; i < NELEMS(sCannotEncoreMoves); i++) {
        if (MOVE_DATA(sCannotEncoreMoves[i]).effect == MOVE_DATA(move).effect) {
            break;
        }
    }

    return i == NELEMS(sCannotEncoreMoves);
}

static const u16 sCannotMeFirstMoves[] = {
    MOVE_COUNTER,
    MOVE_MIRROR_COAT,
    MOVE_THIEF,
    MOVE_COVET,
    MOVE_FOCUS_PUNCH,
    MOVE_CHATTER,
};

BOOL Move_MeFirstCanCopy(BattleContext *battleCtx, u16 move)
{
    int i;
    for (i = 0; i < NELEMS(sCannotMeFirstMoves); i++) {
        if (MOVE_DATA(sCannotMeFirstMoves[i]).effect == MOVE_DATA(move).effect) {
            break;
        }
    }

    return i == NELEMS(sCannotMeFirstMoves);
}

s32 BattleSystem_GetItemData(BattleContext *battleCtx, u16 item, enum ItemDataParam paramID)
{
    u16 dataID = Item_FileID(item, ITEM_FILE_TYPE_DATA);
    ItemData *data = ItemTable_Index(battleCtx->aiContext.itemTable, dataID);

    return Item_Get(data, paramID);
}

int BattleSystem_SideToBattler (BattleSystem * param0, BattleContext * param1, int param2)
{
    int v0;
    int v1;

    v1 = BattleSystem_MaxBattlers(param0);

    for (v0 = 0; v0 < v1; v0++) {
        if (Battler_Side(param0, v0) == param2) {
            break;
        }
    }

    return v0;
}

void BattleSystem_SortMonsInTrickRoom (BattleSystem * param0, BattleContext * param1)
{
    int v0, v1;
    int v2;
    int v3;
    int v4;
    int v5;

    v5 = BattleSystem_MaxBattlers(param0);

    for (v0 = 0; v0 < v5 - 1; v0++) {
        for (v1 = v0 + 1; v1 < v5; v1++) {
            v2 = param1->battlerActionOrder[v0];
            v3 = param1->battlerActionOrder[v1];

            if (param1->battlerActions[v2][3] == param1->battlerActions[v3][3]) {
                if (param1->battlerActions[v2][3] == 1) {
                    v4 = 0;
                } else {
                    v4 = 1;
                }

                if (BattleSystem_CompareBattlerSpeed(param0, param1, v2, v3, v4)) {
                    param1->battlerActionOrder[v0] = v3;
                    param1->battlerActionOrder[v1] = v2;
                }
            }
        }
    }
}

static const enum StatusEffect sEffectsAlwaysShown[] = {
    STATUS_EFFECT_CHANGE_FORM_OUT,
    STATUS_EFFECT_CHANGE_FORM_IN,
    STATUS_EFFECT_ITEM_ESCAPE,
    STATUS_EFFECT_WEATHER_FOG,
    STATUS_EFFECT_WEATHER_RAIN,
    STATUS_EFFECT_WEATHER_HAIL,
    STATUS_EFFECT_WEATHER_SAND,
    STATUS_EFFECT_WEATHER_SUN,
    STATUS_EFFECT_SUBSTITUTE_ON,
    STATUS_EFFECT_SUBSTITUTE_OFF,
};

BOOL BattleSystem_ShouldShowStatusEffect(BattleContext *battleCtx, int battler, int status)
{
    BOOL result = FALSE;

    if ((battleCtx->battleMons[battler].statusVolatile & VOLATILE_CONDITION_SUBSTITUTE)
            || (battleCtx->battleMons[battler].moveEffectsMask & MOVE_EFFECT_SEMI_INVULNERABLE)) {
        for (int i = 0; i < NELEMS(sEffectsAlwaysShown); i++) {
            if (sEffectsAlwaysShown[i] == status) {
                result = TRUE;
                break;
            }
        }
    } else {
        result = TRUE;
    }

    return result;
}

BOOL BattleSystem_TriggerHeldItemOnPivotMove (BattleSystem * param0, BattleContext * param1, int * param2)
{
    BOOL v0;
    int v1;
    int v2;
    int v3;
    int v4;
    int v5;
    int v6;

    v0 = 0;
    v2 = Battler_HeldItemEffect(param1, param1->attacker);
    v3 = Battler_HeldItemPower(param1, param1->attacker, 0);
    v4 = Battler_HeldItemEffect(param1, param1->defender);
    v5 = Battler_HeldItemPower(param1, param1->defender, 0);
    v6 = Battler_Side(param0, param1->attacker);

    if ((v2 == 88) && (param1->battleStatusMask & 0x2000) && (param1->selfTurnFlags[param1->attacker].shellBellDamageDealt) && (param1->attacker != param1->defender) && (param1->battleMons[param1->attacker].curHP < param1->battleMons[param1->attacker].maxHP) && (param1->battleMons[param1->attacker].curHP)) {
        param1->hpCalcTemp = BattleSystem_Divide(param1->selfTurnFlags[param1->attacker].shellBellDamageDealt * -1, v3);
        param1->msgBattlerTemp = param1->attacker;
        param2[0] = (0 + 213);
        v0 = 1;
    }

    if ((v2 == 98) && (Battler_Ability(param1, param1->attacker) != 98) && (param1->battleStatusMask & 0x2000) && (param1->aiContext.moveTable[param1->moveCur].class != 2) && (param1->battleMons[param1->attacker].curHP)) {
        param1->hpCalcTemp = BattleSystem_Divide(param1->battleMons[param1->attacker].maxHP * -1, 10);
        param1->msgBattlerTemp = param1->attacker;
        param2[0] = (0 + 214);
        v0 = 1;
    }

    if ((v4 == 46) && (param1->battleMons[param1->attacker].curHP) && (Battler_Ability(param1, param1->attacker) != 98) && (param1->selfTurnFlags[param1->defender].physicalDamageTaken)) {
        param1->hpCalcTemp = BattleSystem_Divide(param1->battleMons[param1->attacker].maxHP * -1, v5);
        param2[0] = (0 + 266);
        v0 = 1;
    }

    if ((v4 == 116) && (param1->battleMons[param1->attacker].curHP) && (param1->battleMons[param1->attacker].heldItem == 0) && ((param1->sideConditions[v6].knockedOffItemsMask & FlagIndex(param1->selectedPartySlot[param1->attacker])) == 0) && ((param1->selfTurnFlags[param1->defender].physicalDamageTaken) || (param1->selfTurnFlags[param1->defender].specialDamageTaken)) && (param1->aiContext.moveTable[param1->moveCur].flags & 0x1)) {
        param2[0] = (0 + 216);
        v0 = 1;
    }

    return v0;
}

void BattleSystem_DecPPForPressure (BattleContext * param0, int param1, int param2)
{
    if ((param2 != 0xff) && (Battler_Ability(param0, param2) == 46) && (param0->battleMons[param1].ppCur[param0->moveSlot[param1]])) {
        param0->battleMons[param1].ppCur[param0->moveSlot[param1]]--;
    }
}

BOOL BattleSystem_RecordingStopped (BattleSystem *battleSys, BattleContext *battleCtx)
{
    if (ov16_0223F710(battleSys)) {
        battleCtx->command = 42;
        return TRUE;
    }

    return FALSE;
}

int BattleContext_Get(BattleSystem *battleSys, BattleContext *battleCtx, enum BattleContextParam paramID, int battler)
{
    int side;
    switch (paramID) {
    case BATTLECTX_SIDE_CONDITIONS_MASK:
        side = Battler_Side(battleSys, battler);
        return battleCtx->sideConditionsMask[side];

    case BATTLECTX_SIDE_MIST_TURNS:
        side = Battler_Side(battleSys, battler);
        return battleCtx->sideConditions[side].mistTurns;

    case BATTLECTX_SELECTED_PARTY_SLOT:
        return battleCtx->selectedPartySlot[battler];

    case BATTLECTX_TOTAL_TURNS:
        return battleCtx->totalTurns;
        
    case BATTLECTX_LEVEL_UP_MONS:
        return battleCtx->levelUpMons;
        
    case BATTLECTX_SAFARI_ESCAPE_COUNT:
        return battleCtx->safariEscapeCount;
        
    case BATTLECTX_TOTAL_FAINTED_FOR:
        return battleCtx->totalFainted[battler];
        
    case BATTLECTX_TOTAL_DAMAGE_FOR:
        return battleCtx->totalDamage[battler];
        
    case BATTLECTX_ACTION_FOR:
        return battleCtx->battlerActions[battler][BATTLE_ACTION_PICK_COMMAND];
        
    case BATTLECTX_AICTX_DEFENDER:
        return battleCtx->aiContext.defender;
        
    case BATTLECTX_SWITCHING_MASK:
        return battleCtx->battlersSwitchingMask;
        
    case BATTLECTX_AICTX_SELECTED_TARGET:
        return battleCtx->aiContext.selectedTarget[battler];
        
    case BATTLECTX_ACTION_TEMP_FOR:
        return battleCtx->battlerActions[battler][BATTLE_ACTION_TEMP_VALUE];
        
    case BATTLECTX_CONTROL_COMMAND:
        return battleCtx->command;
        
    case BATTLECTX_NEXT_CONTROL_COMMAND:
        return battleCtx->commandNext;
    }

    GF_ASSERT(FALSE);
    return 0;
}

void BattleContext_Set(BattleSystem *battleSys, BattleContext *battleCtx, enum BattleContextParam paramID, int battler, int val)
{
    int side;
    switch (paramID) {
    case BATTLECTX_SIDE_CONDITIONS_MASK:
        side = Battler_Side(battleSys, battler);
        battleCtx->sideConditionsMask[side] = val;
        break;

    case BATTLECTX_SIDE_MIST_TURNS:
        side = Battler_Side(battleSys, battler);
        battleCtx->sideConditions[side].mistTurns = val;
        break;

    case BATTLECTX_SELECTED_PARTY_SLOT:
        battleCtx->selectedPartySlot[battler] = val;
        break;

    case BATTLECTX_TOTAL_TURNS:
        battleCtx->totalTurns = val;
        break;

    case BATTLECTX_AICTX_DEFENDER:
        battleCtx->aiContext.defender = val;
        break;

    case BATTLECTX_AICTX_SELECTED_TARGET:
        battleCtx->aiContext.selectedTarget[battler] = val;
        break;

    default:
        GF_ASSERT(FALSE);
        break;
    }
}

#include "data/move_side_effect_subscripts.h"

/**
 * @brief Map the given side effect to an appropriate subscript.
 * 
 * @param battleCtx 
 * @param type      Type of side effect; see enum SideEffectType
 * @param effect    Effect which should be mapped to a corresponding subscript
 * @return int 
 */
static int MapSideEffectToSubscript(BattleContext *battleCtx, enum SideEffectType type, u32 effect)
{
    battleCtx->sideEffectType = type;
    battleCtx->sideEffectParam = effect & MOVE_SIDE_EFFECT_SUBSCRIPT_POINTER;
    battleCtx->sideEffectFlags = effect & MOVE_SIDE_EFFECT_FLAGS;

    if (effect & MOVE_SIDE_EFFECT_TO_ATTACKER) {
        battleCtx->sideEffectMon = battleCtx->attacker;
    } else if (effect & MOVE_SIDE_EFFECT_TO_DEFENDER) {
        battleCtx->sideEffectMon = battleCtx->defender;
    } else if ((effect & MOVE_SIDE_EFFECT_ON_HIT) || (effect & MOVE_SIDE_EFFECT_CHECK_HP)) {
        battleCtx->sideEffectMon = 0;
    }

    GF_ASSERT(NELEMS(sSideEffectSubscripts) > (effect & MOVE_SIDE_EFFECT_SUBSCRIPT_POINTER));

    return sSideEffectSubscripts[effect & MOVE_SIDE_EFFECT_SUBSCRIPT_POINTER];
}

static int ov16_0225B63C (BattleContext * param0, int param1, int param2, int param3, int param4, u32 * param5)
{
    if (((param0->battleStatusMask & 0x800) == 0) && ((param0->battleStatusMask & 0x8000) == 0) && (param3)) {
        param3 = BattleSystem_Divide(param3 * param2, 10);
    }

    switch (param2) {
    case 0:
        param5[0] |= 0x8;
        param5[0] &= (0x4 ^ 0xffffffff);
        param5[0] &= (0x2 ^ 0xffffffff);
        break;
    case 5:
        if (param4) {
            if (param5[0] & 0x2) {
                param5[0] &= (0x2 ^ 0xffffffff);
            } else {
                param5[0] |= 0x4;
            }
        }
        break;
    case 20:
        if (param4) {
            if (param5[0] & 0x4) {
                param5[0] &= (0x4 ^ 0xffffffff);
            } else {
                param5[0] |= 0x2;
            }
        }
        break;
    }

    return param3;
}

static BOOL ov16_0225B6C8 (BattleContext * param0, int param1)
{
    switch (param0->aiContext.moveTable[param1].effect) {
    case 26:
    case 39:
    case 75:
    case 145:
    case 151:
    case 155:
    case 255:
    case 256:
    case 263:
    case 273:
        return param0->battleStatusMask & 0x200;
        break;
    }

    return 1;
}

/**
 * @brief Get the type of the battler (e.g. Normal, Fighting, etc.).
 * 
 * An additional layer of logic is applied to Arceus to account for Multitype.
 * 
 * @param battleCtx 
 * @param battler 
 * @param paramID   Either BATTLEMON_TYPE_1 or BATTLEMON_TYPE_2
 * @return The battler's type
 */
static u8 Battler_MonType(BattleContext *battleCtx, int battler, enum BattleMonParam paramID)
{
    u8 type;
    if (paramID == BATTLEMON_TYPE_1) {
        type = battleCtx->battleMons[battler].type1;
    } else if (paramID == BATTLEMON_TYPE_2) {
        type = battleCtx->battleMons[battler].type2;
    } else {
        GF_ASSERT(FALSE);
    }

    if (battleCtx->battleMons[battler].species == SPECIES_ARCEUS
            && battleCtx->battleMons[battler].ability == ABILITY_MULTITYPE) {
        switch (BattleSystem_GetItemData(battleCtx, battleCtx->battleMons[battler].heldItem, ITEM_PARAM_HOLD_EFFECT)) {
        case HOLD_EFFECT_ARCEUS_FIRE:
            type = TYPE_FIRE;
            break;

        case HOLD_EFFECT_ARCEUS_WATER:
            type = TYPE_WATER;
            break;

        case HOLD_EFFECT_ARCEUS_ELECTRIC:
            type = TYPE_ELECTRIC;
            break;

        case HOLD_EFFECT_ARCEUS_GRASS:
            type = TYPE_GRASS;
            break;

        case HOLD_EFFECT_ARCEUS_ICE:
            type = TYPE_ICE;
            break;

        case HOLD_EFFECT_ARCEUS_FIGHT:
            type = TYPE_FIGHTING;
            break;

        case HOLD_EFFECT_ARCEUS_POISON:
            type = TYPE_POISON;
            break;

        case HOLD_EFFECT_ARCEUS_GROUND:
            type = TYPE_GROUND;
            break;

        case HOLD_EFFECT_ARCEUS_FLYING:
            type = TYPE_FLYING;
            break;

        case HOLD_EFFECT_ARCEUS_PSYCHIC:
            type = TYPE_PSYCHIC;
            break;

        case HOLD_EFFECT_ARCEUS_BUG:
            type = TYPE_BUG;
            break;

        case HOLD_EFFECT_ARCEUS_ROCK:
            type = TYPE_ROCK;
            break;

        case HOLD_EFFECT_ARCEUS_GHOST:
            type = TYPE_GHOST;
            break;

        case HOLD_EFFECT_ARCEUS_DRAGON:
            type = TYPE_DRAGON;
            break;

        case HOLD_EFFECT_ARCEUS_DARK:
            type = TYPE_DARK;
            break;

        case HOLD_EFFECT_ARCEUS_STEEL:
            type = TYPE_STEEL;
            break;

        default:
            type = TYPE_NORMAL;
            break;
        }
    }

    return type;
}

/**
 * @brief Clear the AI's knowledge of any moves for the given battler.
 * 
 * @param battleCtx 
 * @param battler 
 */
static void BattleAI_ClearKnownMoves(BattleContext *battleCtx, u8 battler)
{
    for (int i = 0; i < LEARNED_MOVES_MAX; i++) {
        battleCtx->aiContext.battlerMoves[battler][i] = MOVE_NONE;
    }
}

/**
 * @brief Clear the AI's knowledge of the given battler's ability.
 * 
 * @param battleCtx 
 * @param battler 
 */
static void BattleAI_ClearKnownAbility(BattleContext *battleCtx, u8 battler)
{
    battleCtx->aiContext.battlerAbilities[battler] = ABILITY_NONE;
}

/**
 * @brief Clear the AI's knowledge of the given battler's held item.
 * 
 * @param battleCtx 
 * @param battler 
 */
static void BattleAI_ClearKnownItem(BattleContext *battleCtx, u8 battler)
{
    battleCtx->aiContext.battlerHeldItems[battler] = ITEM_NONE;
}

/**
 * @brief Choose the target for Trace.
 * 
 * @param battleSys 
 * @param battleCtx 
 * @param defender1 
 * @param defender2 
 * @return Either of defender1 or defender2, if at least one has a Traceable
 * ability, or BATTLER_NONE, if there is no valid Trace target.
 */
static int ChooseTraceTarget(BattleSystem *battleSys, BattleContext *battleCtx, int defender1, int defender2)
{
    int trace = BATTLER_NONE;

    if (battleCtx->battleMons[defender1].ability != ABILITY_FORECAST
            && battleCtx->battleMons[defender1].ability != ABILITY_TRACE
            && battleCtx->battleMons[defender1].ability != ABILITY_MULTITYPE
            && battleCtx->battleMons[defender1].curHP
            && battleCtx->battleMons[defender2].curHP
            && battleCtx->battleMons[defender2].ability != ABILITY_FORECAST
            && battleCtx->battleMons[defender2].ability != ABILITY_TRACE
            && battleCtx->battleMons[defender2].ability != ABILITY_MULTITYPE) {
        // Both targets are eligible; choose randomly
        if (BattleSystem_RandNext(battleSys) & 1) {
            trace = defender2;
        } else {
            trace = defender1;
        }
    } else if (battleCtx->battleMons[defender1].ability != ABILITY_FORECAST
            && battleCtx->battleMons[defender1].ability != ABILITY_TRACE
            && battleCtx->battleMons[defender1].curHP
            && battleCtx->battleMons[defender1].ability != ABILITY_MULTITYPE) {
        trace = defender1;
    } else if (battleCtx->battleMons[defender2].ability != ABILITY_FORECAST
            && battleCtx->battleMons[defender2].ability != ABILITY_TRACE
            && battleCtx->battleMons[defender2].curHP
            && battleCtx->battleMons[defender2].ability != ABILITY_MULTITYPE) {
        trace = defender2;
    }

    return trace;
}

static const u16 sMovesCannotTriggerAnticipation[] = {
    BATTLE_EFFECT_40_DAMAGE_FLAT,
    BATTLE_EFFECT_LEVEL_DAMAGE_FLAT,
    BATTLE_EFFECT_RANDOM_DAMAGE_1_TO_150_LEVEL,
    BATTLE_EFFECT_COUNTER,
    BATTLE_EFFECT_MIRROR_COAT,
    BATTLE_EFFECT_METAL_BURST,
};

static BOOL MoveCannotTriggerAnticipation(BattleContext *battleCtx, int move)
{
    for (int i = 0; i < NELEMS(sMovesCannotTriggerAnticipation); i++) {
        if (sMovesCannotTriggerAnticipation[i] == MOVE_DATA(move).effect) {
            return TRUE;
        }
    }

    return FALSE;
}

static int BattleMove_Type (BattleSystem * param0, BattleContext * param1, int param2, int param3)
{
    int v0;

    switch (param3) {
    case 363:
        v0 = Battler_NaturalGiftType(param1, param2);
        break;
    case 449:
        switch (Battler_HeldItemEffect(param1, param2)) {
        case 131:
            v0 = 1;
            break;
        case 134:
            v0 = 2;
            break;
        case 132:
            v0 = 3;
            break;
        case 133:
            v0 = 4;
            break;
        case 137:
            v0 = 5;
            break;
        case 136:
            v0 = 6;
            break;
        case 138:
            v0 = 7;
            break;
        case 141:
            v0 = 8;
            break;
        case 126:
            v0 = 10;
            break;
        case 127:
            v0 = 11;
            break;
        case 129:
            v0 = 12;
            break;
        case 128:
            v0 = 13;
            break;
        case 135:
            v0 = 14;
            break;
        case 130:
            v0 = 15;
            break;
        case 139:
            v0 = 16;
            break;
        case 140:
            v0 = 17;
            break;
        default:
            v0 = 0;
            break;
        }
        break;
    case 237:
        v0 = ((param1->battleMons[param2].hpIV & 1) >> 0) | ((param1->battleMons[param2].attackIV & 1) << 1) | ((param1->battleMons[param2].defenseIV & 1) << 2) | ((param1->battleMons[param2].speedIV & 1) << 3) | ((param1->battleMons[param2].spAttackIV & 1) << 4) | ((param1->battleMons[param2].spDefenseIV & 1) << 5);
        v0 = (v0 * 15 / 63) + 1;

        if (v0 >= 9) {
            v0++;
        }
        break;
    case 311:
        if ((BattleSystem_CountAbility(param0, param1, 8, 0, 13) == 0) && (BattleSystem_CountAbility(param0, param1, 8, 0, 76) == 0)) {
            if (param1->fieldConditionsMask & (0x3 | 0xc | 0x30 | 0xc0 | 0x8000)) {
                if (param1->fieldConditionsMask & 0x3) {
                    v0 = 11;
                }

                if (param1->fieldConditionsMask & 0xc) {
                    v0 = 5;
                }

                if (param1->fieldConditionsMask & 0x30) {
                    v0 = 10;
                }

                if (param1->fieldConditionsMask & 0xc0) {
                    v0 = 15;
                }
            }
        }
        break;
    default:
        v0 = 0;
        break;
    }

    return v0;
}

int ov16_0225BA88 (BattleSystem * param0, int param1)
{
    int v0, v1;
    u8 v2;
    u8 v3;
    u8 v4;
    u8 v5;
    u8 v6;
    u16 v7;
    u16 v8;
    int v9;
    u8 v10;
    u8 v11;
    u8 v12;
    u8 v13 = 6;
    u8 v14, v15;
    u32 v16;
    int v17, v18;
    Pokemon * v19;
    BattleContext * v20;

    v20 = BattleSystem_Context(param0);
    v14 = param1;

    if ((BattleSystem_BattleType(param0) & 0x10) || (BattleSystem_BattleType(param0) & 0x8)) {
        v15 = v14;
    } else {
        v15 = BattleSystem_Partner(param0, param1);
    }

    v2 = BattleSystem_RandomOpponent(param0, v20, param1);
    v17 = 0;
    v18 = BattleSystem_PartyCount(param0, param1);
    v10 = 0;

    while (v10 != 0x3f) {
        v12 = 0;
        v13 = 6;

        for (v0 = v17; v0 < v18; v0++) {
            v19 = BattleSystem_PartyPokemon(param0, param1, v0);
            v7 = Pokemon_GetValue(v19, MON_DATA_SPECIES_EGG, NULL);

            if ((v7 != 0) && (v7 != 494) && (Pokemon_GetValue(v19, MON_DATA_CURRENT_HP, NULL)) && ((v10 & FlagIndex(v0)) == 0) && (v20->selectedPartySlot[v14] != v0) && (v20->selectedPartySlot[v15] != v0) && (v0 != v20->aiSwitchedPartySlot[v14]) && (v0 != v20->aiSwitchedPartySlot[v15])) {
                v3 = BattleMon_Get(v20, v2, 27, NULL);
                v4 = BattleMon_Get(v20, v2, 28, NULL);
                v5 = Pokemon_GetValue(v19, MON_DATA_TYPE_1, NULL);
                v6 = Pokemon_GetValue(v19, MON_DATA_TYPE_2, NULL);
                v11 = BattleSystem_TypeMatchupMultiplier(v5, v3, v4);
                v11 += BattleSystem_TypeMatchupMultiplier(v6, v3, v4);

                if (v12 < v11) {
                    v12 = v11;
                    v13 = v0;
                }
            } else {
                v10 |= FlagIndex(v0);
            }
        }

        if (v13 != 6) {
            v19 = BattleSystem_PartyPokemon(param0, param1, v13);

            for (v0 = 0; v0 < 4; v0++) {
                v8 = Pokemon_GetValue(v19, MON_DATA_MOVE1 + v0, NULL);
                v9 = ov16_0225BE3C(param0, v20, v19, v8);

                if (v8) {
                    v16 = 0;
                    ov16_022552D4(v20, v8, v9, Pokemon_GetValue(v19, MON_DATA_ABILITY, NULL), Battler_Ability(v20, v2), Battler_HeldItemEffect(v20, v2), BattleMon_Get(v20, v2, 27, NULL), BattleMon_Get(v20, v2, 28, NULL), &v16);

                    if (v16 & 0x2) {
                        break;
                    }
                }
            }

            if (v0 == 4) {
                v10 |= FlagIndex(v13);
            } else {
                return v13;
            }
        } else {
            v10 = 0x3f;
        }
    }

    v12 = 0;
    v13 = 6;

    for (v0 = v17; v0 < v18; v0++) {
        v19 = BattleSystem_PartyPokemon(param0, param1, v0);
        v7 = Pokemon_GetValue(v19, MON_DATA_SPECIES_EGG, NULL);

        if ((v7 != 0) && (v7 != 494) && (Pokemon_GetValue(v19, MON_DATA_CURRENT_HP, NULL)) && (v20->selectedPartySlot[v14] != v0) && (v20->selectedPartySlot[v15] != v0) && (v0 != v20->aiSwitchedPartySlot[v14]) && (v0 != v20->aiSwitchedPartySlot[v15])) {
            for (v1 = 0; v1 < 4; v1++) {
                v8 = Pokemon_GetValue(v19, MON_DATA_MOVE1 + v1, NULL);
                v9 = ov16_0225BE3C(param0, v20, v19, v8);

                if ((v8) && (v20->aiContext.moveTable[v8].power != 1)) {
                    v11 = BattleSystem_CalcMoveDamage(param0, v20, v8, v20->sideConditionsMask[Battler_Side(param0, v2)], v20->fieldConditionsMask, 0, 0, param1, v2, 1);
                    v16 = 0;
                    v11 = BattleSystem_CheckTypeChart(param0, v20, v8, v9, param1, v2, v11, &v16);

                    if (v16 & (0x8 | 0x800 | 0x100000 | 0x40000)) {
                        v11 = 0;
                    }
                }

                if (v12 < v11) {
                    v12 = v11;
                    v13 = v0;
                }
            }
        }
    }

    return v13;
}

int ov16_0225BE28 (BattleSystem * param0, int param1)
{
    BattleContext * v0;

    v0 = BattleSystem_Context(param0);
    return v0->aiSwitchedPartySlot[param1];
}

int ov16_0225BE3C (BattleSystem * param0, BattleContext * param1, Pokemon * param2, int param3)
{
    int v0;

    switch (param3) {
    case 363:
        v0 = BattleSystem_GetItemData(param1, Pokemon_GetValue(param2, MON_DATA_HELD_ITEM, NULL), 12);
        break;
    case 449:
        switch (BattleSystem_GetItemData(param1, Pokemon_GetValue(param2, MON_DATA_HELD_ITEM, NULL), 1)) {
        case 131:
            v0 = 1;
            break;
        case 134:
            v0 = 2;
            break;
        case 132:
            v0 = 3;
            break;
        case 133:
            v0 = 4;
            break;
        case 137:
            v0 = 5;
            break;
        case 136:
            v0 = 6;
            break;
        case 138:
            v0 = 7;
            break;
        case 141:
            v0 = 8;
            break;
        case 126:
            v0 = 10;
            break;
        case 127:
            v0 = 11;
            break;
        case 129:
            v0 = 12;
            break;
        case 128:
            v0 = 13;
            break;
        case 135:
            v0 = 14;
            break;
        case 130:
            v0 = 15;
            break;
        case 139:
            v0 = 16;
            break;
        case 140:
            v0 = 17;
            break;
        default:
            v0 = 0;
            break;
        }
        break;
    case 237:
        v0 = ((Pokemon_GetValue(param2, MON_DATA_HP_IV, NULL) & 1) >> 0) | ((Pokemon_GetValue(param2, MON_DATA_ATK_IV, NULL) & 1) << 1) | ((Pokemon_GetValue(param2, MON_DATA_DEF_IV, NULL) & 1) << 2) | ((Pokemon_GetValue(param2, MON_DATA_SPEED_IV, NULL) & 1) << 3) | ((Pokemon_GetValue(param2, MON_DATA_SPATK_IV, NULL) & 1) << 4) | ((Pokemon_GetValue(param2, MON_DATA_SPDEF_IV, NULL) & 1) << 5);
        v0 = (v0 * 15 / 63) + 1;

        if (v0 >= 9) {
            v0++;
        }
        break;
    case 311:
        if ((BattleSystem_CountAbility(param0, param1, 8, 0, 13) == 0) && (BattleSystem_CountAbility(param0, param1, 8, 0, 76) == 0)) {
            if (param1->fieldConditionsMask & (0x3 | 0xc | 0x30 | 0xc0 | 0x8000)) {
                if (param1->fieldConditionsMask & 0x3) {
                    v0 = 11;
                }

                if (param1->fieldConditionsMask & 0xc) {
                    v0 = 5;
                }

                if (param1->fieldConditionsMask & 0x30) {
                    v0 = 10;
                }

                if (param1->fieldConditionsMask & 0xc0) {
                    v0 = 15;
                }
            }
        }
        break;
    default:
        v0 = 0;
        break;
    }

    return v0;
}
