#pragma once
#include <QString>
#include <QTime>
#include <QBitArray>
#include <bitset>
#include <QRandomGenerator>


#define LEN_TYPE123 168
#define LEN_TYPE5 424
namespace AIS_Data_Type {

    typedef struct{
        unsigned int typeMessage;
        unsigned int MMSI;
    }BaseAis;


    struct ClassA123 : BaseAis{
        int navigation =0;         // состояние навигации
        int ROT=0;                // скорость поворота
        int SOG=0;                // скорость относительно земли
        int PositionAccuracy=0;   // точность положения
        double lon=0;             // долгота
        double lat=0;             // широта
        double COG=0;             // курс относительно земли
        unsigned int HDG=0;                // истинное направление от 0 до 359, недоступно-511
        unsigned int time=60;               // отметка времени
        int DTE=0;                // индикатор манёвра
        int RAIM=0;               // индикатор манёвра


        static inline void calculatePos(ClassA123 &data){
            // Генерируем случайные изменения в диапазоне [-5, 5]
            int deltaCOG = QRandomGenerator::global()->bounded(-5, 6);
            int deltaSOG = QRandomGenerator::global()->bounded(-5, 6);
            int deltaHDG = QRandomGenerator::global()->bounded(-5, 6);
            int hdg = data.HDG;
            // Обновляем COG и обеспечиваем, что он остается в диапазоне [0, 359]
            data.COG += deltaCOG;
            if (data.COG < 0) data.COG += 360;
            if (data.COG >= 360) data.COG -= 360;

            // Обновляем HDG и обеспечиваем, что он остается в диапазоне [0, 359]
            hdg += deltaHDG;
            if (hdg < 0) hdg += 360;
            if (hdg >= 360) hdg -= 360;
            data.HDG = hdg;
            // Обновляем SOG и обеспечиваем, что он остается неотрицательным
            data.SOG += deltaSOG;
            if (data.SOG < 0) data.SOG = 0;

            // Преобразование углов в радианы
            double cogRadians = qDegreesToRadians(data.COG);

            // Расстояние, пройденное за время deltaTime (в километрах)
            double distanceTravelled = (data.SOG * 60) / 3600.0 * 1.852; // Узлы -> Км/ч -> Км

            // Рассчитываем новые координаты
            double deltaLat = (distanceTravelled / 111.32) * qSin(cogRadians);
            double deltaLon = (distanceTravelled / (111.32 * qCos(qDegreesToRadians(data.lat)))) * qCos(cogRadians);

            // Обновляем координаты
            data.lat += deltaLat;
            data.lon += deltaLon;

            // Ограничение широты и долготы в допустимых пределах
            if (data.lat > 90.0) data.lat = 90.0;
            if (data.lat < -90.0) data.lat = -90.0;
            if (data.lon > 180.0) data.lon -= 360.0;
            if (data.lon < -180.0) data.lon += 360.0;
        }
    };


    struct ClassA5 : BaseAis{

        unsigned int    IMO;                // номер IMO
        QString    CallSign;                // позывной
        QString    VesselName;              // наименование судна
        unsigned int        ShipType;                // тип судна
        unsigned int    DimensionBow;    // размерности -до носа
        unsigned int    DimensionStern;     //             -до кормы
        unsigned int    DimensionPort;      //             -до левого борта
        unsigned int    DimensionStarboard; //             -до правого борта
        int    PositionType;                // тип системы позиционирования
        // char    ETAmonth;                // дата прибытия  -месяц
        // char    ETAday;                  //                -день
        // char    ETAhour;                 //                -час
        // char    ETAminute;               //                -минута
        QDateTime   ETA;
        double     Draught;                     // осадка
        QString    Destination;             // место назначения 20- шестибитных символов
        unsigned int    DTE;
    };

    struct ParamClassA {
        ClassA123 t123;
        ClassA5 t5;
    };

};

// Функция для преобразования числа в бинарную строку фиксированной длины
inline std::string toBinaryString(unsigned int number, int length) {
    std::string binary = std::bitset<32>(number).to_string();
    // Убираем лишние нули слева
    binary = binary.substr(32 - length);
    return binary;
}

// Функция для кодирования значения в биты
inline void encodeValueBytes(std::vector<bool> &bitField, unsigned int value, int start, int end) {
    std::string binary = toBinaryString(value, end - start + 1);
    for (int i = 0; i < binary.length(); ++i) {
        bitField[start + i] = (binary[i] == '1');
    }
}

// // // Функция для кодирования строки в биты
// inline void encodeAsciiBytes(std::vector<bool> &bitField, const std::string value, int start, int end, size_t requiredLength) {
//     // Проверяем длину строки и дополняем её символами '@', если она меньше требуемой длины
//     std::string paddedValue = value;
//     if (paddedValue.length() < requiredLength) {
//         paddedValue.append(requiredLength - paddedValue.length(), '@');
//     } else if (paddedValue.length() > requiredLength) {
//         paddedValue = paddedValue.substr(0, requiredLength); // Обрезаем до требуемой длины
//     }
    
//     int index = 0;
//     for (size_t i = start; i < end && index < paddedValue.length(); i += 6) {
//         char byte = paddedValue[index++];
//         unsigned int sixBitsValue = (byte >= '@') ? (byte - 64) : byte;
//         std::string sixBitsBinary = toBinaryString(sixBitsValue, 6);
//         for (int j = 0; j < 6 && (i + j) < end; ++j) {
//             bitField[i + j] = (sixBitsBinary[j] == '1');
//         }
//     }
// }

inline unsigned char calculateChecksum(const QString &message) {
    unsigned char checksum = 0;
    for (int i = 1; i < message.length(); ++i) { // Начинаем с 1, чтобы пропустить '!'
        checksum ^= static_cast<unsigned char>(message.at(i).toLatin1());
    }
    return checksum;
}

namespace AIS_NMEA_Builder {
    template<class T>
    class BaseNmeaString {
    public:
        BaseNmeaString();
        ~BaseNmeaString();
        void setParamets(const T &params);
        QStringList getString();
        QString encodeString(const std::vector<bool> &bitField, int lenStr);
    
    protected:
        void get_checksum(std::string data, char *result);
        virtual  QString decodeParam() = 0;
        T paramets;
    private:
        
    };

    template<class T>
    BaseNmeaString<T>::BaseNmeaString(){
    }

    template<class T>
    BaseNmeaString<T>::~BaseNmeaString(){
    }

    template<class T>
    void BaseNmeaString<T>::setParamets(const T &params){
        paramets = params;
    }


    template<class T>
    QStringList BaseNmeaString<T>::getString(){
        QString encodedMessage = decodeParam();
        QStringList nmeaMessages;
        const int maxPayloadLength = 82 - 5; // Максимальная длина полезной нагрузки в символах (учитываем теги и запятые)
        int payloadLength = encodedMessage.length();
        int fragmentCount = (payloadLength + maxPayloadLength - 1) / maxPayloadLength; // Округление вверх

        for (int fragmentNumber = 1; fragmentNumber <= fragmentCount; ++fragmentNumber) {
            int start = (fragmentNumber - 1) * maxPayloadLength;
            int end = qMin(start + maxPayloadLength, payloadLength);
            QString payloadFragment = encodedMessage.mid(start, end - start);

            QString nmeaMessage = "!AIVDM," +
                                QString::number(fragmentCount) + "," +
                                QString::number(fragmentNumber) + "," +
                                (fragmentCount > 1 ? "1" : "") + "," + // Message ID (пустой для одного фрагмента)
                                "B," + // Channel code (B для канала B)
                                payloadFragment + "," +
                                "0"; // Fill bits (предполагаем, что нет дополнительных бит)

            unsigned char checksum = calculateChecksum(nmeaMessage);
            nmeaMessage += "*" + QString::number(checksum, 16).toUpper().rightJustified(2, '0') + "\r\n";;
            nmeaMessages.append(nmeaMessage);
        }

        return nmeaMessages;
    }
    template<class T>
    QString BaseNmeaString<T>::encodeString(const std::vector<bool> &bitField, int lenStr){
        std::string encodedMessage;
        for (int i = 0; i < lenStr; i += 6) {
            unsigned int sixBitsValue = 0;
            for (int j = 0; j < 6; ++j) {
                if (i + j < lenStr && bitField[i + j]) {
                    sixBitsValue |= (1 << (5 - j));
                }
            }
            if (sixBitsValue <= 40) {
                encodedMessage += static_cast<char>(sixBitsValue + 48); // '0'-'9', ':', ';', '<', '=', '>', '?'
            } else if (sixBitsValue > 40 && sixBitsValue <= 63) {
                encodedMessage += static_cast<char>(sixBitsValue + 48 - 8 + ('@' - '0')); // '@', 'A'-'W'
            } else {
                throw std::out_of_range("Six-bit value is out of range");
            }
        }
        return QString::fromStdString( encodedMessage);
    }

    class Type123Decoder : public BaseNmeaString<AIS_Data_Type::ClassA123 >{
        public:
            Type123Decoder();
            ~Type123Decoder();
        
        protected:
          virtual  QString decodeParam() override;

    };

    class Type5Decoder : public BaseNmeaString<AIS_Data_Type::ClassA5 >{
        public:
            Type5Decoder();
            ~Type5Decoder();
        
        protected:
          virtual  QString decodeParam() override;

    };
};
