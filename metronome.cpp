#include <iostream>
#include <vector>
#include <set>
#include <tonc.h>

// Initialize the metronome
void init_metro(int bpm)
{
    int seconds_per_minute = 60;
    // Based on common denominator of CPU clock speed
    int ticks_per_second = 16384;
    // Convert musical beat values to CPU tick values. (bpm actually stands for bars for minute here)
    // Negative values cause timer to underflow
    // The value of a bar in CPU ticks, negative because this causes timer to underflow at max value - beat
    int beat = -1 * seconds_per_minute * ticks_per_second / bpm;

    // Set timer settings to create a 6/8 time signature (2 timers used with different subdivisions)
    // Subdivision for melody (8th notes)
    REG_TM0D= beat / 6;
    REG_TM0CNT= TM_FREQ_1024 | TM_ENABLE;
    // Cascade causes timer 1 to increment after timer 0 overflows (i.e. after 1/6 of a bar has passed)
    REG_TM1CNT= TM_ENABLE | TM_CASCADE;
    // Subdivision for percussion (16th notes or 1/12 of a bar)
    REG_TM2D= beat / 12;
    REG_TM2CNT= TM_FREQ_1024 | TM_ENABLE;
    // Cascade causes timer 3 to increment after timer 2 overflows (i.e. after 1/12 of a bar has passed)
    REG_TM3CNT= TM_ENABLE | TM_CASCADE;
}

void init_sound()
{
    // Setup and enable sound settings on the Gameboy with different timbres
    REG_SNDSTAT= SSTAT_ENABLE;
    REG_SNDDMGCNT= SDMG_BUILD(SDMG_SQR1 | SDMG_NOISE, SDMG_SQR1 | SDMG_SQR2, 7, 7);
    REG_SNDDSCNT= SDS_DMG100;
    REG_SND1SWEEP= SSW_BUILD(3, 1, 4);
    REG_SND1CNT= SSQR_ENV_BUILD(7, 0, 5) | SSQR_DUTY1_2;
    REG_SND2CNT= SSQR_ENV_BUILD(7, 0, 2) | SSQR_DUTY1_2;
    REG_SND4CNT= SSQR_ENV_BUILD(4, 0, 4);
}

void set_kick_timbre(int timbre) 
{
    if (timbre == 1)
    {
        REG_SND1FREQ = SFREQ_RESET;
        REG_SND1CNT= SSQR_ENV_BUILD(5, 0, 5) | SSQR_DUTY1_2;
    }
    else
    {
        REG_SND1CNT= SSQR_ENV_BUILD(7, 0, 5) | SSQR_DUTY1_2;
    }
}

void init_display()
{
    // Set the display mode to display a solid background
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

template<typename S>
auto select_random(const S &s, size_t n) {
    auto it = std::begin(s);
    std::advance(it,n);
    return it;
}

std::vector<int> generatePrimeZero(std::string phrase)
{
    // Generate tone rows based on input string ascii values
    // Fill the rest of the row with random values
    std::vector<int> p_zero;
    std::set<int> pitch_set = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    for (int i=0; i<12; i++)
    {
        if (i < phrase.size() && pitch_set.contains(phrase[i] % 12))
        {
            p_zero.push_back(phrase[i] % 12);
            pitch_set.erase(phrase[i] % 12);
        }
        else
        {
            if(pitch_set.size() == 0)
                break;
            auto r = rand() % pitch_set.size();
            auto n = *select_random(pitch_set, r);
            p_zero.push_back(n);
            pitch_set.erase(n);
        }
    }
    return p_zero;
}

void print_status(bool isInverted, bool isRetrograde, int row_index)
{
    tte_erase_screen();
    init_display();
    std::cout << "\n\n\n\n\n\n";
    std::cout << "Hello, world!" << "\n";
    std::cout << (isInverted ? "Playing: Inverted Row " : "Playing: Prime Row ") << row_index << "\n" << "Retrograde: " <<  (isRetrograde ? "On" : "Off") << "\n";
}

int main() {
    // First timer value (note melody)
    int sec = -1;
    // Second timer value (percussion)
    int secc = -1;
    // Note subdivision index
    int index = 0;
    // Indices and settings for current tone row being played
    int row_index = 0;
    bool isInverted = false;
    bool isRetrograde = false;
    // Percussion subdivision index
    int indexx = 0;
    // note array for the snare
    int beat[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0};
    // note array for the kick (2 hits)
    int kick[12] = {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0};
    // Second kick hit is a lower pitch (different timbre)
    int kick_timbre[12] = {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0};
    // Array for octaves of each beat, makes switching octaves convenient
    int octaves[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    init_display();
    init_metro(70);
    init_sound();

    std::vector<int> p_zero = generatePrimeZero("Hello world!");
    int first_row[p_zero.size()];
    int tone_row[p_zero.size()][p_zero.size()];
    int i = 0;
    int adding[p_zero.size()];
    // Calculate the rest of the tone row matrix
    // Similar to: https://www.musictheory.net/calculators/matrix
    while(i < p_zero.size())
    {
        first_row[i] = p_zero.at(i);
        adding[i] = first_row[0] - first_row[i];
        i++;
    }
    for(int a=0;a<p_zero.size();a++)
    {
        for(int b=0;b<p_zero.size();b++)
        {
            if(first_row[b] < adding[a])
            {
                tone_row[a][b] = (first_row[b] + 12 + adding[a]) % 12;
            }
            else
            {
                tone_row[a][b] = (first_row[b] + adding[a]) % 12;
            }
            if(tone_row[a][b] < 0)
                tone_row[a][b] += 12;
        }
    }

    print_status(isInverted, isRetrograde, row_index);

    while(1)
    {
        vid_vsync();
        key_poll();
        if(key_hit(KEY_A))
        {
            isInverted = true;
            print_status(isInverted, isRetrograde, row_index);
        }
        if(key_hit(KEY_B))
        {
            isInverted = false;
            print_status(isInverted, isRetrograde, row_index);
        }
        if(key_hit(KEY_LEFT) && row_index > 0)
        {
            row_index -= 1;
            print_status(isInverted, isRetrograde, row_index);
        }
        if(key_hit(KEY_RIGHT) && row_index < 11)
        {
            row_index += 1;
            print_status(isInverted, isRetrograde, row_index);
        }
        if(key_hit(KEY_UP))
        {
            isRetrograde = true;
            print_status(isInverted, isRetrograde, row_index);
        }
        if(key_hit(KEY_DOWN))
        {
            isRetrograde = false;
            print_status(isInverted, isRetrograde, row_index);
        }
        // When overflows happen (i.e. when the timer value changes), play each note accordingly
        if(REG_TM1D != sec)
        {
            sec = REG_TM1D;
            index = isRetrograde ? 11 - (sec % 12) : sec % p_zero.size();
            play(tone_row[isInverted ? index : row_index][isInverted ? row_index : index], 1);
        }
        if(REG_TM3D != secc)
        {
            secc = REG_TM3D;
            indexx = secc % p_zero.size();
            bool hit = beat[indexx];
            bool another_hit = kick[indexx];
            set_kick_timbre(kick_timbre[indexx]);
            play_beat(hit);
            play_kick(another_hit);
        }
    }
    return 0;
}
