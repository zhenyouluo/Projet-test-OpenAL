#include <iostream>
#include <vector>
#include <string.h>
#include <string>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <sndfile.h>

// Fonction d'initialisation d'openal
bool InitOpenAL();

// Fonction récuperrant la liste des devices
void GetDevices(std::vector<std::string>& devices);

// Fonction de destruction
void ShutdownOpenAL();

// Fonction permettant de charger un son avec libsndfile et création du buffer
ALuint LoadSound(const std::string& fileName);

// Fonction permmetant de lire le son
void PlaySound(ALuint buffer);

int main(int argc, char** argv){

    if(argc<2)
        return -1;

    const std::string song(argv[1]);

    InitOpenAL();
    ALuint buffer = LoadSound(song);
    PlaySound(buffer);

    ShutdownOpenAL();

    return 0;
}

bool InitOpenAL(){

    ALCdevice* device = alcOpenDevice(NULL);
    // Ouverture du device
    if (!device)
        return false;

    // Création du contexte
    ALCcontext* context = alcCreateContext(device, NULL);
    if (!context)
        return false;

    // Activation du contexte
    if (!alcMakeContextCurrent(context))
        return false;

    return true;
}

void GetDevices(std::vector<std::string>& devices){

    // Vidage de la liste
    devices.clear();

    // Récupération des devices disponibles
    const ALCchar* deviceList = alcGetString(NULL, ALC_DEVICE_SPECIFIER);

    if (deviceList){
        while(strlen(deviceList) > 0){
            devices.push_back(deviceList);
            deviceList += strlen(deviceList) + 1;
        }
    }

}


void ShutdownOpenAL(){

    // Récupération du contexte et du device
    ALCcontext* context = alcGetCurrentContext();
    ALCdevice* device = alcGetContextsDevice(context);

    // Désactivation du contexte
    alcMakeContextCurrent(NULL);

    // Destruction du contexte
    alcDestroyContext(context);

    // Fermeture du device
    alcCloseDevice(device);

}


ALuint LoadSound(const std::string& fileName){

    // Ouverture du fichier audio avec libsndfile
    SF_INFO fileInfos;
    SNDFILE* file = sf_open(fileName.c_str(), SFM_READ, &fileInfos);
    if(!file)
        return 0;
    ALsizei nbSamples = static_cast<ALsizei>(fileInfos.channels * fileInfos.frames);
    ALsizei sampleRate = static_cast<ALsizei>(fileInfos.samplerate);

    // Lecture des échantillons audio au format entier 16 bits signé (le plus commun)
    std::vector<ALshort> samples(nbSamples);
    if (sf_read_short(file, &samples[0], nbSamples) < nbSamples)
        return 0;

    // Fermeture du fichier
    sf_close(file);

    // Détermination du format en fonction du nombre de canaux
    ALenum format;
    switch (fileInfos.channels){
        case 1 : format = AL_FORMAT_MONO16; break;
        case 2 : format = AL_FORMAT_STEREO16; break;
        default : return 0;
    }

    // Création du tampon OpenAL
    ALuint buffer;
    alGenBuffers(1, &buffer);

    // Remplissage avec les échantillons lus
    alBufferData(buffer, format, &samples[0], nbSamples*sizeof(ALushort), sampleRate);

    // Vérification des erreurs
    if (alGetError() != AL_NO_ERROR)
        return 0;

    return buffer;
}

void PlaySound(ALuint buffer){

    // Création d'une source
    ALuint source;
    alGenSources(1, &source);

    // On attache le tampon contenant les échantillons audio à la source
    alSourcei(source, AL_BUFFER, buffer);

    // Lecture du son
    alSourcePlay(source);

    ALint status;
    do{
        // Récupération et affichage de la position courant de lecture en secondes
        ALfloat seconds = 0.f;
        alGetSourcef(source, AL_SEC_OFFSET, &seconds);
        std::cout<<"\rLeture en cours..."<<std::fixed<<seconds<<" sec";

        // Récupération de l'état du son
        alGetSourcei(source, AL_SOURCE_STATE, &status);
    }
    while(status == AL_PLAYING);

    // Destruction du tampon
    alDeleteBuffers(1, &buffer);

    // Destruction de la source
    alSourcei(source, AL_BUFFER, 0);
    alDeleteSources(1, &source);
}
