int main()
{
//*****************************************
//Program g³ówny

//Wczytywanie pliku
AudioFile<double> audioFile;
audioFile.load ("./phrase.wav");

//Zmienne
int channel = 0;
int numSamples = audioFile.getNumSamplesPerChannel();
int oldFrequency = audioFile.getSampleRate();
int newFrequency = 0, newRealFrequency = 0;
unsigned long long int N = 0, D = 0;
double roundedFrequency = 0;

//Dane odczytane z pliku
std::cout << "Czestotliwosc wczytanego pliku: " << oldFrequency << std::endl;

//Wczytanie nowej czêstotliwoœci
std::cout << "Podaj nowa czestotliwosc: "; std::cin >> newFrequency;

//Obliczanie funkcji round - zaokr¹glenie do dwóch miejsc po przecinku
if(newFrequency < oldFrequency) roundedFrequency = round(((double)newFrequency / (double)oldFrequency) * 100) / 100;
else if(newFrequency > oldFrequency) roundedFrequency = round(((double)oldFrequency / (double)newFrequency) * 100) / 100;
std::cout << "Stosunek czestotliwosci: " << roundedFrequency << std::endl;

//Obliczanie funkcji rat - wymierne przedstawienie liczby za pomoc¹ ilorazu liczb ca³kowitych
int accuracy = 8;
long double x[accuracy], y[accuracy];
double temp = roundedFrequency;
double temp_t = roundedFrequency;
x[0] = floor(temp);
for(int i = 1; i < accuracy; i++)
{
    temp = 1.0 / (temp_t - x[i - 1]);
    if(temp < 0) temp*= -1;
    temp_t = temp;
    x[i] = floor(temp);
    if(x[i] > 100000)
    {
        accuracy = i;
        break;
    }
}
y[accuracy - 1] = x[accuracy - 1];
y[accuracy - 2] = x[accuracy - 2] * x[accuracy - 1] + 1;
for(int i = accuracy - 3; i > -1; i--)
{
    y[i] = x[i] * y[i + 1] + y[i + 2];
}
if(newFrequency < oldFrequency)
{
    N = y[0];
    D = y[1];
}
else if(newFrequency > oldFrequency)
{
    N = y[1];
    D = y[0];
}
else
{
    N = 1; D = 1;
}
std::cout << "N = " << N << ", D = " << D << std::endl;

//Obliczanie nowej czêstotliwoœci
newRealFrequency = (oldFrequency * N) / D;
std::cout << "Nowa czestotliwosc: " << newRealFrequency << std::endl;

// Wczytanie wektora próbek z pliku do naszego wektora
vector<complex<double>> X;
for(int i = 0; i < audioFile.getNumSamplesPerChannel(); i++)
{
    X.push_back(audioFile.samples[0][i]);
}

//*********************************************
//Funkcja NADMIAROWE

//Sprawdzenie przystoœci iloœci próbek
int length = X.size();
if(length % 2 == 1)
{
    X.push_back(0);
    length = X.size();
}

//Wype³nianie nowego wektora zerami
vector<complex<double>> Y;
for(int i = 0; i < length * N; i++)
{
    Y.push_back(0);
}

//Wpisywanie próbki z X co N-próbek w Y
for(int i = 0; i < length * N; i++)
{
    if(i % N == 0) Y[i] = X[i / N];
}

//Tworzenie wektora maski - okno prostok¹tne
vector<complex<double>> MASKA;
for(int i = 0; i < ((length * N) / (2 * N)); i++)
{
    MASKA.push_back(1);
}
for(int i = 0; i < ((length * N) - ((length * N) / N)); i++)
{
    MASKA.push_back(0);
}
for(int i = 0; i < ((length * N) / (2 * N)); i++)
{
    MASKA.push_back(1);
}

//Obliczanie wektora wyjœciowego (1 - FFT, 2 - MASKA, 3 - IFFT, 4 - czêœæ rzeczywista)
vector<complex<double>> out_n;
Fft::transform(Y);
for(int i = 0; i < length * N; i++)
{
    Y[i] = Y[i] * MASKA[i];
}
Fft::invTransform(Y);
for(int i = 0; i < length * N; i++)
{
    out_n.push_back(real(Y[i]));
}

//*****************************************************
//Funkcja PASMOWE

//Sprawdzanie rozmiaru wektora
length = out_n.size();
if(length % (2 * D) != 0)
{
    for(int i = 0; i < (2 * D - (length % (2 * D))); i++)
    {
        out_n.push_back(0);
    }
    length = out_n.size();
}

//Funkcja MASKA 2
MASKA.clear();
for(int i = 0; i < length / (2 * D); i++)
{
    MASKA.push_back(1);
}
for(int i = 0; i < length - length / D; i++)
{
    MASKA.push_back(0);
}
for(int i = 0; i < length / (2 * D); i++)
{
    MASKA.push_back(1);
}

//Obliczanie wektora wyjœciowego (1 - FFT, 2 - Maska, 3 - IFFT, 4 - czêœæ rzeczywista)
vector<double> out_p;
Fft::transform(out_n);

for(int i = 0; i < length; i++)
{
    out_n[i] = out_n[i] * MASKA[i];
}
Fft::invTransform(out_n);
for(int i = 0; i < length; i++)
{
    out_p.push_back(real(out_n[i]));
}
audioFile.clearAudioBuffer();
AudioFile<double>::AudioBuffer buffer;
buffer.resize(1);
vector<double> out;
for(int i = 0; i < length; i++)
{
    if(i % D == 0) out.push_back(out_p[i]);
}
length = out.size();
buffer[0].resize(length);
for(int i = 0; i < length; i++)
{
    buffer[0][i] = 20 * out[i];
}
bool ok = audioFile.setAudioBuffer(buffer);

//Ustawienie nowej czêstotliwoœci i zapis do nowego pliku
audioFile.setSampleRate(newRealFrequency);
audioFile.save ("./new_phrase.wav");
}