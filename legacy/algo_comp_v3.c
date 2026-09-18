/*
* Taka Khoo
* ALGO_COMP.C
* A Really Simple Algorithmic Composition Generator
*
* This program uses randomness and user input to generate a musical composition.
* It writes a Csound score file (.csd) with instrument definitions and score events,
* then calls Csound to play the composition (and optionally render it to an audio file).
* A time-stretched version can also be produced.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// Convert a note step (1-8) into a frequency (Hz) based on the selected scale.
float StepToHerz(int scale, int thisnote) {
    float thisnotehz;
    switch(scale) {
        case 1: // Ionian (Major)
            printf("Scale: Ionian\n");
            switch(thisnote) {
                case 1: thisnotehz = 261.63; break; // C4
                case 2: thisnotehz = 293.66; break; // D4
                case 3: thisnotehz = 329.63; break; // E4
                case 4: thisnotehz = 349.23; break; // F4
                case 5: thisnotehz = 392.00; break; // G4
                case 6: thisnotehz = 440.00; break; // A4
                case 7: thisnotehz = 493.88; break; // B4
                case 8: thisnotehz = 523.25; break; // C5
                default: thisnotehz = 261.63; break;
            }
            break;
        case 2: // Dorian
            printf("Scale: Dorian\n");
            switch(thisnote) {
                case 1: thisnotehz = 261.63; break;
                case 2: thisnotehz = 293.66; break;
                case 3: thisnotehz = 311.13; break; // Eb4
                case 4: thisnotehz = 349.23; break;
                case 5: thisnotehz = 392.00; break;
                case 6: thisnotehz = 440.00; break;
                case 7: thisnotehz = 466.16; break; // Bb4
                case 8: thisnotehz = 523.25; break;
                default: thisnotehz = 261.63; break;
            }
            break;
        case 3: // Phrygian
            printf("Scale: Phrygian\n");
            switch(thisnote) {
                case 1: thisnotehz = 261.63; break;
                case 2: thisnotehz = 277.18; break; // Db4
                case 3: thisnotehz = 311.13; break; // Eb4
                case 4: thisnotehz = 349.23; break;
                case 5: thisnotehz = 392.00; break;
                case 6: thisnotehz = 415.30; break; // Ab4
                case 7: thisnotehz = 466.16; break; // Bb4
                case 8: thisnotehz = 523.25; break;
                default: thisnotehz = 261.63; break;
            }
            break;
        case 4: // Lydian
            printf("Scale: Lydian\n");
            switch(thisnote) {
                case 1: thisnotehz = 261.63; break;
                case 2: thisnotehz = 293.66; break;
                case 3: thisnotehz = 329.63; break;
                case 4: thisnotehz = 369.99; break; // F#4
                case 5: thisnotehz = 392.00; break;
                case 6: thisnotehz = 440.00; break;
                case 7: thisnotehz = 493.88; break;
                case 8: thisnotehz = 523.25; break;
                default: thisnotehz = 261.63; break;
            }
            break;
        case 5: // Mixolydian
            printf("Scale: Mixolydian\n");
            switch(thisnote) {
                case 1: thisnotehz = 261.63; break;
                case 2: thisnotehz = 293.66; break;
                case 3: thisnotehz = 329.63; break;
                case 4: thisnotehz = 349.23; break;
                case 5: thisnotehz = 392.00; break;
                case 6: thisnotehz = 440.00; break;
                case 7: thisnotehz = 466.16; break;
                case 8: thisnotehz = 523.25; break;
                default: thisnotehz = 261.63; break;
            }
            break;
        case 6: // Aeolian
            printf("Scale: Aeolian\n");
            switch(thisnote) {
                case 1: thisnotehz = 261.63; break;
                case 2: thisnotehz = 293.66; break;
                case 3: thisnotehz = 311.13; break;
                case 4: thisnotehz = 349.23; break;
                case 5: thisnotehz = 392.00; break;
                case 6: thisnotehz = 415.30; break;
                case 7: thisnotehz = 466.16; break;
                case 8: thisnotehz = 523.25; break;
                default: thisnotehz = 261.63; break;
            }
            break;
        case 7: // Locrian
            printf("Scale: Locrian\n");
            switch(thisnote) {
                case 1: thisnotehz = 261.63; break;
                case 2: thisnotehz = 277.18; break;
                case 3: thisnotehz = 311.13; break;
                case 4: thisnotehz = 349.23; break;
                case 5: thisnotehz = 369.99; break;
                case 6: thisnotehz = 415.30; break;
                case 7: thisnotehz = 466.16; break;
                case 8: thisnotehz = 523.25; break;
                default: thisnotehz = 261.63; break;
            }
            break;
        default:
            printf("Invalid scale input. Defaulting to Ionian.\n");
            thisnotehz = 261.63;
            break;
    }
    return thisnotehz;
}

int main(void) {
    // Variable declarations
    int i = 0, rep = 0;
    int randnotes[256];
    int x = 0;
    int thisnote;
    float thisnotehz;
    srand((unsigned)time(0));
    int notediff;
    int flag = 2;
    FILE *csdPointer;
    char *csdFileName;
    int scale;
    float totaltime = 0;
    int BPM;
    double notespeed;
    int division;
    int flagend;
    float time;       // Section duration (seconds)
    float currenttime = 0;
    char scalename[12];
    // Arrays for storing sequence info (for transposition, if needed)
    // Hi-hat variables
    float notespeedhh;
    int divisionpercuarray[256];
    int divisionpercu = 10;
    float totaltimehh;
    float openorclosed;
    // Kick drum variables (not fully used here)
    int dacorsf;
    
    // -------------------------------
    // Get user input
retry:
    printf("============================================\n");
    printf("Enter musical mode (1: Ionian, 2: Dorian, 3: Phrygian, 4: Lydian, 5: Mixolydian, 6: Aeolian, 7: Locrian): ");
    scanf("%d", &scale);
    if(scale < 1 || scale > 7) {
        printf("Error: Invalid scale type. Try again.\n");
        goto retry;
    }
    printf("\nEnter BPM (20-200, recommended 30-100): ");
    scanf("%d", &BPM);
    if(BPM < 20 || BPM > 200) {
        printf("Error: Invalid BPM. Try again.\n");
        goto retry;
    }
    printf("\nEnter section duration (seconds): ");
    scanf("%f", &time);
    if(time <= 0.0 || time > 1000.0) {
        printf("Error: Invalid time. Try again.\n");
        goto retry;
    }
    printf("\nEnter number of sections: ");
    scanf("%d", &i);  // We'll use i as the section count here
    if(i < 1 || i > 100) {
        printf("Error: Invalid number of sections. Try again.\n");
        goto retry;
    }
    int sections = i; // Save total sections
    printf("\nEnter number of loops per section (1-10): ");
    scanf("%d", &rep);
    if(rep < 1 || rep > 10) {
        printf("Error: Invalid loop count. Try again.\n");
        goto retry;
    }
    printf("\nRender to sound file? (0 = No, 1 = Yes): ");
    scanf("%d", &dacorsf);
    if(dacorsf < 0 || dacorsf > 1) {
        printf("Error: Invalid entry. Try again.\n");
        goto retry;
    }
    
    // -------------------------------
    // Create the Csound score file (.csd)
    csdFileName = "algo_comp_v3.csd";
    csdPointer = fopen(csdFileName, "w");
    if(csdPointer != NULL) {
        // Write global Csound settings
        fprintf(csdPointer, "<CsoundSynthesizer>\n");
        fprintf(csdPointer, "sr = 44100\n");
        fprintf(csdPointer, "kr = 4410\n");
        fprintf(csdPointer, "ksmps = 10\n");
        fprintf(csdPointer, "nchnls = 2\n\n");
        
        // Instrument definitions
        fprintf(csdPointer, "<CsInstruments>\n");
        fprintf(csdPointer, "garvbL init 0\n");
        fprintf(csdPointer, "garvbR init 0\n\n");
        
        // Instrument 1: Melody
        fprintf(csdPointer, "instr 1\n");
        fprintf(csdPointer, "ims = 1000\n");
        fprintf(csdPointer, "k1 linsegr 0, 0.002, 1, p3/4, 1, p3/4, 0\n");
        fprintf(csdPointer, "a1 oscili 4000, p4, 1\n");
        fprintf(csdPointer, "out a1 * k1\n");
        fprintf(csdPointer, "garvbL = garvbL + (0.5 * a1 * k1)\n");
        fprintf(csdPointer, "garvbR = garvbR + (0.5 * a1 * k1)\n");
        fprintf(csdPointer, "endin\n\n");
        
        // Instrument 2: Hi-Hat (Noise)
        fprintf(csdPointer, "instr 2\n");
        fprintf(csdPointer, "kamp = 200\n");
        fprintf(csdPointer, "kbeta linsegr 0.5, 0.001, 0, p5, 0.5\n");
        fprintf(csdPointer, "k1 linsegr 0, 0.001, 1, p5, 0\n");
        fprintf(csdPointer, "a1 noise kamp, kbeta\n");
        fprintf(csdPointer, "out a1 * k1\n");
        fprintf(csdPointer, "garvbL = garvbL + (0.5 * a1 * k1)\n");
        fprintf(csdPointer, "garvbR = garvbR + (0.5 * a1 * k1)\n");
        fprintf(csdPointer, "endin\n\n");
        
        // Instrument 3: Bass Drum
        fprintf(csdPointer, "instr 3\n");
        fprintf(csdPointer, "k1 linseg 0, 0.0001, 1, 0.2, 0\n");
        fprintf(csdPointer, "k2 linseg 200, 0.05, 55\n");
        fprintf(csdPointer, "a1 oscili 3000, k2, 2\n");
        fprintf(csdPointer, "out a1 * k1\n");
        fprintf(csdPointer, "endin\n");
        
        // Instrument 99: Reverb and output
        fprintf(csdPointer, "instr 99\n");
        fprintf(csdPointer, "denorm garvbL\n");
        fprintf(csdPointer, "denorm garvbR\n");
        fprintf(csdPointer, "aoutL, aoutR reverbsc garvbL, garvbR, 0.95, 15000, 44100, 0.4\n");
        fprintf(csdPointer, "outs aoutL, aoutR\n");
        fprintf(csdPointer, "clear garvbL\n");
        fprintf(csdPointer, "clear garvbR\n");
        fprintf(csdPointer, "endin\n\n");
        
        fprintf(csdPointer, "</CsInstruments>\n");
        fprintf(csdPointer, "<CsScore>\n\n");
        
        // Write function tables
        fprintf(csdPointer, "f 1 0 8192 10 1 1/3 1/5 1/7 1/9 1/11 1/13 1/15\n");
        fprintf(csdPointer, "f 2 0 8192 10 1\n");
        fprintf(csdPointer, "f 3 0 8192 10 1 0.6 0.5 0.4 0.3 0.2 0.1\n");
        
        // -------------------------------
        // For each section, generate the melody and (if applicable) hi-hat patterns.
        // totaltime accumulates the overall composition time.
        for (int sec = 0; sec < sections; sec++) {
            printf("Section %d\n", sec);
            currenttime = 0;
            // Generate a sequence of random notes for this section
            for (i = 0; i < 256; ) {
                thisnote = (rand() % 8) + 1;
                randnotes[i] = thisnote;
                if(i > 0)
                    notediff = randnotes[i] - randnotes[i - 1];
                else
                    notediff = 0;
                if(notediff > 5 || notediff < -5) {
                    flag = 1;  // Too large interval; retry this note.
                } else {
                    flag = 2;
                }
                if(flag == 2) i++;
            }
            
            printf("Generating melody for section %d...\n", sec);
            i = 0;
            // Generate melody events until the section duration is reached.
            while(currenttime < time) {
                thisnote = randnotes[i];
                // Determine if we need to force cadence (last notes)
                if(currenttime >= (time - (120.0/BPM)) && currenttime < (time - (70.0/BPM))) {
                    flagend = 2;
                    thisnotehz = 392.00; // Dominant (G4)
                } else if(currenttime > (time - (60.0/BPM))) {
                    flagend = 2;
                    thisnotehz = 261.63; // Root (C4)
                    notespeed = 1.0;
                } else if(currenttime == 0) {
                    thisnotehz = 261.63; // Start on C4
                    flagend = 2;
                } else if(thisnotehz == 392.00) {
                    thisnotehz = 261.63;
                    flagend = 2;
                } else {
                    flagend = 1;
                }
                if(flagend == 1) {
                    thisnotehz = StepToHerz(scale, thisnote);
                } else if(flagend == 2) {
                    // Cadence condition met; skip random conversion.
                    goto score;
                }
                printf("Processing melody note at %.2f sec...\n", currenttime);
            score:
                division = rand() % 7;
                switch(division) {
                    case 0: notespeed = 60.0/BPM; break;
                    case 1: notespeed = 60.0/BPM; break;
                    case 2: notespeed = 60.0/(BPM*2); break;
                    case 3: notespeed = 60.0/(BPM*2); break;
                    case 4: notespeed = 60.0/(BPM*4); break;
                    case 5: notespeed = 60.0/(BPM*4); break;
                    case 6: notespeed = 60.0/(BPM/2); break;
                }
                if(currenttime == (time - (60.0/BPM)))
                    notespeed = 1.0;
                fprintf(csdPointer, "i1 %f %f %f\n", totaltime, notespeed, thisnotehz);
                totaltime += notespeed;
                currenttime += notespeed;
                i++;
            }
            
            // -------------------------------
            // Generate hi-hat pattern for sections beyond the first
            if(sec >= 1) {
                printf("Generating hi-hat pattern for section %d...\n", sec);
                // Determine section start time (each section is 'time' seconds)
                float sectionStart = totaltime - time;
                totaltimehh = sectionStart;
                x = 0;
                int sixthflag = 0;
                // Loop until we reach the end of this section
                do {
                    divisionpercuarray[x] = rand() % 4;
                    // Apply a simple pattern rule: if the previous two divisions were '2', force a '1'
                    if(x >= 2 && divisionpercuarray[x-2] == 2 && divisionpercuarray[x-1] == 2) {
                        divisionpercuarray[x] = 1;
                    }
                    // If divisionpercu is 3, count it and then adjust
                    if(divisionpercu == 3) {
                        divisionpercuarray[x] = 3;
                        sixthflag++;
                    }
                    if(sixthflag == 4) {
                        sixthflag = 0;
                        divisionpercuarray[x] = 0;
                    }
                    divisionpercu = divisionpercuarray[x];
                    switch(divisionpercu) {
                        case 0: notespeedhh = 60.0/BPM; openorclosed = 0.1; break;
                        case 1: notespeedhh = 60.0/(BPM*2); openorclosed = 0.05; break;
                        case 2: notespeedhh = 60.0/(BPM*4); openorclosed = 0.02; break;
                        case 3: notespeedhh = 60.0/(BPM*8); openorclosed = 0.02; break;
                    }
                    printf("Hi-hat division: %d\n", divisionpercuarray[x]);
                    fprintf(csdPointer, "i2 %f %f 1 %f\n", totaltimehh, notespeedhh/100, openorclosed);
                    totaltimehh += notespeedhh;
                    x++;
                } while(totaltimehh < totaltime); // End hi-hat loop when we reach end of section
            }
            
            // Optionally, repeat a transposed melody (octave up) for looping (not fully implemented)
            printf("Repeating transposed melody %d times for section %d...\n", rep, sec);
            // [Loop code for transposed melody would go here if implemented]
        }
        
        // Composition summary and add reverb tail
        switch(scale) {
            case 1: strncpy(scalename, "Ionian", 12); break;
            case 2: strncpy(scalename, "Dorian", 12); break;
            case 3: strncpy(scalename, "Phrygian", 12); break;
            case 4: strncpy(scalename, "Lydian", 12); break;
            case 5: strncpy(scalename, "Mixolydian", 12); break;
            case 6: strncpy(scalename, "Aeolian", 12); break;
            case 7: strncpy(scalename, "Locrian", 12); break;
        }
        printf("\nBPM = %d, Total runtime = %.2f seconds\n", BPM, time * sections);
        printf("Mode = %s\n", scalename);
        
        fprintf(csdPointer, "i99 0 %f\n", totaltime + 10.0);
        fprintf(csdPointer, "</CsScore>\n");
        fprintf(csdPointer, "</CsoundSynthesizer>\a");
        fclose(csdPointer);
        
        // -------------------------------
        // Play the generated Csound score file
        char commandline[1000] = "csound -odac -d -O null ";
        strcat(commandline, csdFileName);
        if(dacorsf == 1) {
            char cmd_2[1000] = " && csound -o composed.aif ";
            strcat(cmd_2, csdFileName);
            strcat(commandline, cmd_2);
        }
        if (system("command -v csound >/dev/null 2>&1") != 0) {
            fprintf(
                stderr,
                "Csound was not found on PATH; generated %s without playback.\n",
                csdFileName
            );
        } else if (system(commandline) != 0) {
            fprintf(stderr, "Csound exited with an error. The generated score remains at %s.\n", csdFileName);
        }
    }
    
    // Option for time-stretched version
    int flag2;
    printf("\nDo you want a time-stretched version? (0 = Stop, 1 = Timestretch, 2 = Try again): ");
    scanf("%d", &flag2);
    if(flag2 == 1) {
        float stretchfactor;
        printf("\nEnter time stretch factor (1 = normal, 2 = twice as long): ");
        scanf("%f", &stretchfactor);
        FILE* csdpointer2;
        char* csdFileName2 = "algo_comp_v3_stretched.csd";
        csdpointer2 = fopen(csdFileName2, "w");
        if(csdpointer2 != NULL) {
            fprintf(csdpointer2, "<CsoundSynthesizer>\n");
            fprintf(csdpointer2, "<CsInstruments>\n");
            fprintf(csdpointer2, "sr = 44100\n");
            fprintf(csdpointer2, "ksmps = 32\n");
            fprintf(csdpointer2, "0dbfs = 1\n");
            fprintf(csdpointer2, "nchnls = 2\n\n");
            fprintf(csdpointer2, "garvbL init 0\n");
            fprintf(csdpointer2, "garvbR init 0\n\n");
            fprintf(csdpointer2, "instr 1\n");
            fprintf(csdpointer2, "ktimewarp = %f\n", stretchfactor);
            fprintf(csdpointer2, "kresample init 1\n");
            fprintf(csdpointer2, "ibeg = 0\n");
            fprintf(csdpointer2, "iwsize = 4410\n");
            fprintf(csdpointer2, "irandw = 882\n");
            fprintf(csdpointer2, "itimemode = 0\n");
            fprintf(csdpointer2, "ioverlap = p4\n");
            fprintf(csdpointer2, "asig sndwarp .5, ktimewarp, kresample, 1, ibeg, iwsize, irandw, ioverlap, 2, itimemode\n");
            fprintf(csdpointer2, "outs (asig*0.3), (asig*0.3)\n");
            fprintf(csdpointer2, "garvbL = garvbL + (0.05 * asig)\n");
            fprintf(csdpointer2, "garvbR = garvbR + (0.05 * asig)\n");
            fprintf(csdpointer2, "endin\n\n");
            fprintf(csdpointer2, "instr 99\n");
            fprintf(csdpointer2, "denorm garvbL\n");
            fprintf(csdpointer2, "denorm garvbR\n");
            fprintf(csdpointer2, "aoutL, aoutR reverbsc garvbL, garvbR, 0.95, 15000, 44100, 0.4\n");
            fprintf(csdpointer2, "outs aoutL, aoutR\n");
            fprintf(csdpointer2, "clear garvbL\n");
            fprintf(csdpointer2, "clear garvbR\n");
            fprintf(csdpointer2, "endin\n\n");
            fprintf(csdpointer2, "</CsInstruments>\n");
            fprintf(csdpointer2, "<CsScore>\n\n");
            fprintf(csdpointer2, "f 1 0 %lf 1 \"composed.aif\" 0 0 0\n", totaltime * 44100);
            fprintf(csdpointer2, "f 2 0 1024 9 0.5 1 0\n");
            fprintf(csdpointer2, "i 1 0 %f 15\n", totaltime * stretchfactor);
            fprintf(csdpointer2, "i99 0 %f\n", (totaltime * stretchfactor) + 10);
            fprintf(csdpointer2, "</CsScore>\n");
            fprintf(csdpointer2, "</CsoundSynthesizer>\a");
            fclose(csdpointer2);
            
            char commandline2[1000] = "csound -odac -d -O null ";
            strcat(commandline2, csdFileName2);
            system(commandline2);
        }
    } else if(flag2 == 0) {
        return 0;
    } else if(flag2 == 2) {
        goto retry;
    }
    return 0;
}
