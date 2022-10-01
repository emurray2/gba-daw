#include <iostream>
#include <tonc.h>

void init_metro(int bpm)
{
    int seconds_per_minute = 60;
    int ticks_per_second = 16384;
    int beat = -1 * seconds_per_minute * ticks_per_second / bpm;

    REG_TM0D= beat / 4;
    REG_TM0CNT= TM_FREQ_1024 | TM_ENABLE;
    REG_TM1CNT= TM_ENABLE | TM_CASCADE;
    REG_TM2D= beat / 4;
    REG_TM2CNT= TM_FREQ_1024 | TM_ENABLE;
    REG_TM3CNT= TM_ENABLE | TM_CASCADE;
}

void init_sound()
{
    REG_SNDSTAT= SSTAT_ENABLE;
    REG_SNDDMGCNT= SDMG_BUILD(SDMG_SQR1 | SDMG_NOISE, SDMG_SQR1 | SDMG_SQR2, 7, 7);
    REG_SNDDSCNT= SDS_DMG100;
    REG_SND1SWEEP= SSW_BUILD(3, 1, 4);
    REG_SND1CNT= SSQR_ENV_BUILD(10, 0, 4) | SSQR_DUTY1_2;
    REG_SND2CNT= SSQR_ENV_BUILD(8, 0, 2) | SSQR_DUTY1_2;
    REG_SND4CNT= SSQR_ENV_BUILD(8, 0, 5);
}

void init_display()
{
    REG_DISPCNT= DCNT_MODE0 | DCNT_BG0;

    tte_init_se_default(0, BG_CBB(0)|BG_SBB(31));
    tte_init_con();
}

void play(int note, int octave) {
    REG_SND2FREQ = SFREQ_RESET | SND_RATE(note, octave);
}

void play_beat(bool isPlaying) {
    if (isPlaying)
    {
        REG_SND4FREQ = SFREQ_RESET | 0x8032;
    }
}

void play_kick(bool isPlaying) {
    if (isPlaying)
    {
        REG_SND1FREQ = SFREQ_RESET | SND_RATE(NOTE_C, -1);
    }
}

int main() {
    int sec = -1;
    int secc = -1;
    int index = 0;
    int indexx = 0;
    int notes[12] = {0, 3, 7, 11, 0, 11, 7, 3, 0, 3, 7, 11};
    int beat[12] = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1};
    int kick[12] = {1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0};
    int octaves[12] = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};

    init_display();
    init_metro(100.0);
    init_sound();

    while(1)
    {
        if(REG_TM1D != sec)
        {
            sec = REG_TM1D;
            index = sec % 12;
            play(notes[index], octaves[index]);
        }
        if(REG_TM3D != secc)
        {
            secc = REG_TM3D;
            indexx = secc % 12;
            bool hit = beat[indexx];
            bool another_hit = kick[indexx];
            play_beat(hit);
            play_kick(another_hit);
        }
    }
    return 0;
}
