#ifndef HSCCHARACTER_H
#define HSCCHARACTER_H

#include <QColor>
#include <QDomDocument>
#include <QMap>
#include <QObject>

#include <variant>

class hscCharacter
{
public:
    hscCharacter();

    void load(QDomDocument&);

    static QString str(QChar* qcp) { return QString(qcp); }

    class BasicConfiguration {
    public:
        QString basePoints  = "";
        QString experience  = "";
        QString disadPoints = "";

        void load(QDomDocument& xml);
    };

    class CharacterInfo {
    public:
        QString alternateIdentities = "";
        QString hairColor           = "";
        QString playerName          = "";
        QString height              = "";
        QString genre               = "";
        QString characterName       = "";
        QString eyeColor            = "";
        QString gm                  = "";
        QString campaignName        = "";
        QString weight              = "";
        QString background          = "";
        QString personality         = "";
        QString quote               = "";
        QString tactics             = "";
        QString campaignUse         = "";
        QString appearance          = "";

        void load(QDomDocument& xml);
    };

    class Base {
    public:
        Base() = default;
        virtual ~Base() = default;

        QMap<QString, QString> attributes;

        static bool Bool(const QString& s) { return s == "Yes"; }

        static QColor Color(const QString& clr) {
            auto rgb = clr.split(' ');
            if (rgb.count() != 3) return QColor();
            return QColor(rgb[0].toInt(), rgb[1].toInt(), rgb[2].toInt());
        }

        virtual void load(QDomElement& e);
    };

    class Adder: public Base {
    public:
        Adder() = default;
        virtual ~Adder() = default;
    };

    class Modifier: public Base {
    public:
        Modifier() = default;
        virtual ~Modifier() = default;
    };

    template <class T> class Items {
    public:
        QList<T> list;

        auto begin()         { return list.begin(); }
        int  count() const   { return list.count(); }
        auto end()           { return list.end(); }
        bool isEmpty() const { return list.isEmpty(); }

        virtual void load(QDomElement& e) {
            int n = list.count();
            list.emplaceBack(T());
            list[n].load(e);
        }

        void load(QDomDocument& xml, QString x, QString y) {
            QDomElement chr = xml.firstChildElement("CHARACTER");
            QDomElement conf = chr.firstChildElement(x);

            QDomElement e = conf.firstChildElement(y);
            list.clear();
            while (!e.isNull()) {
                load(e);
                e = e.nextSiblingElement(y);
            }
        }

        void load(QDomDocument& xml, QString x) { load(xml, x, x.left(x.length() - 1)); }
    };

    class Characteristics {
    public:
        class Characteristic: public Base {
        public:
            Characteristic() = default;
            virtual ~Characteristic() = default;
            int getLevels() { return attributes["LEVELS"].toInt(); }
        };

        QMap<QString, Characteristic> characteristics;

        void load(QDomDocument& xml) {
            QDomElement chtr = xml.firstChildElement("CHARACTER");
            QDomElement chr = chtr.firstChildElement("CHARACTERISTICS");

            QDomElement e = chr.firstChildElement();
            while (!e.isNull()) {
                characteristics[e.tagName()].load(e);
                e = e.nextSiblingElement();
            }
        }
    };

    class SubSpecials {
    public:
        QList<Adder>    adders;
        QList<Modifier> modifiers;

        virtual void load(QDomElement& spc) {
            QDomElement e = spc.firstChildElement("ADDER");
            while (!e.isNull()) {
                int n = adders.count();
                adders.emplaceBack(Adder());
                adders[n].load(e);
                e = e.nextSiblingElement("ADDER");
            }

            e = spc.firstChildElement("MODIFIER");
            while (!e.isNull()) {
                int n = modifiers.count();
                modifiers.emplaceBack(Modifier());
                modifiers[n].load(e);
                e = e.nextSiblingElement("MODIFIER");
            }
        }
    };

    class SubStat: public Base {
    public:
        SubStat() = default;
        virtual ~SubStat() = default;

        SubSpecials specials;

        void load(QDomElement& stt) override {
            Base::load(stt);
            specials.load(stt);
        }
    };

    class SubPower: public Base {
    public:
        SubPower() = default;
        virtual ~SubPower() = default;

        SubSpecials specials;

        void load(QDomElement& pwr) override {
            Base::load(pwr);
            specials.load(pwr);
        }
    };

    class Specials {
    public:
        QList<Adder>    adders;
        QList<Modifier> modifiers;
        QList<SubPower> subPowers;

        virtual void load(QDomElement& spc) {
            QDomElement e = spc.firstChildElement("ADDER");
            while (!e.isNull()) {
                int n = adders.count();
                adders.emplaceBack(Adder());
                adders[n].load(e);
                e = e.nextSiblingElement("ADDER");
            }

            e = spc.firstChildElement("MODIFIER");
            while (!e.isNull()) {
                int n = modifiers.count();
                modifiers.emplaceBack(Modifier());
                modifiers[n].load(e);
                e = e.nextSiblingElement("MODIFIER");
            }

            e = spc.firstChildElement("POWER");
            while (!e.isNull()) {
                int n = subPowers.count();
                subPowers.emplaceBack(SubPower());
                subPowers[n].load(e);
                e = e.nextSiblingElement("POWER");
            }
        }
    };

    class Skills {
    public:
        class Skill: public Base {
        public:
            Skill() = default;
            virtual ~Skill() = default;

            Specials specials;

            void load(QDomElement& skl) override {
                Base::load(skl);
                specials.load(skl);
            }
        };

        Items<Skill> list;
    };

    class Stat: public Base {
    public:
        Stat() = default;
        virtual ~Stat() = default;

        Specials specials;

        void load(QDomElement& stt) override {
            Base::load(stt);
            specials.load(stt);
        }
    };

    class Perks {
    public:
        class Perk: public Base {
        public:
            Perk() = default;
            virtual ~Perk() = default;

            Specials        specials;

            void load(QDomElement& prk) override {
                Base::load(prk);
                specials.load(prk);
            }
        };

        Items<Perk> list;
    };

    class Talents {
    public:
        class Talent: public Base {
        public:
            Talent() = default;
            virtual ~Talent() = default;

            Specials specials;

            void load(QDomElement& tal) override {
                Base::load(tal);
                specials.load(tal);
            }
        };

        Items<Talent> list;

        int getPrimaries(const QString& stat);
        int getSecondaries(const QString& stat);
    };

    class MartialArts {
    public:
        class MartialArt: public Base {
        public:
            MartialArt() = default;
            virtual ~MartialArt() = default;

            Specials specials;

            void load(QDomElement& ma) override {
                Base::load(ma);
                specials.load(ma);
            }
        };

        Items<MartialArt> list;
    };

    class Power: public Base {
    public:
        Power() = default;
        virtual ~Power() = default;

        Specials specials;

        void load(QDomElement& pwr) override {
            Base::load(pwr);
            specials.load(pwr);
        }
    };

    class Container: public Base {
    public:
        Container() = default;
        virtual ~Container() = default;

        QList<QString> contents;

        void append(QString id) { contents.append(id); }
    };

    class List: public Container {
    public:
        List() = default;
        virtual ~List() = default;

        Specials specials;

        void load(QDomElement& lst) override {
            Base::load(lst);
            specials.load(lst);
        }
    };

    class MultiPower: public Container {
    public:
        MultiPower() = default;
        virtual ~MultiPower() = default;

        Specials specials;
        void load(QDomElement& mp) override {
            Base::load(mp);
            specials.load(mp);
        }
    };

    class VariablePowerPool: public Container {
    public:
        VariablePowerPool() = default;
        virtual ~VariablePowerPool() = default;

        Specials specials;

        void load(QDomElement& vpp) override {
            Base::load(vpp);
            specials.load(vpp);
        }
    };

    class Powers {
    public:
        QMap<QString, std::variant<List, MultiPower, VariablePowerPool>> containers;
        QList<std::variant<Power, Stat, List, MultiPower, VariablePowerPool>> items;

        void load(QDomDocument& xml) {
            QDomElement chr = xml.firstChildElement("CHARACTER");
            QDomElement pow = chr.firstChildElement("POWERS");

            QDomElement e = pow.firstChildElement();
            while (!e.isNull()) {
                if        (e.tagName() == "POWER") {
                    Power power;
                    power.load(e);
                    items.push_back(power);
                } else if (e.tagName() == "LIST") {
                    List list;
                    list.load(e);
                    containers[list.attributes["ID"]] = list;
                    items.push_back(list);
                } else if (e.tagName() == "MULTIPOWER") {
                    MultiPower mp;
                    mp.load(e);
                    containers[mp.attributes["ID"]] = mp;
                    items.push_back(mp);
                } else if (e.tagName() == "VPP") {
                    VariablePowerPool vpp;
                    vpp.load(e);
                    containers[vpp.attributes["ID"]] = vpp;
                    items.push_back(vpp);
                } else {
                    Stat stat;
                    stat.load(e);
                    items.push_back(stat);
                }
                e = e.nextSiblingElement();
            }
        }

        int getPrimaries(const QString& power, const QString& where);
        int getPrimaries(const QString& stat);
        int getSecondaries(const QString& stat);
        int getSecondaries(const QString& power, const QString& where);
    };

    class Disadvantages {
    public:
        class Disadvantage: public Base {
        public:
            Disadvantage() = default;
            virtual ~Disadvantage() = default;

            Specials specials;

            void load(QDomElement& dis) override {
                Base::load(dis);
                specials.load(dis);
            }
        };

        Items<Disadvantage> list;
    };

    class Equipment {
    public:
        void load(QDomDocument&) {
        }
    };

    class Image {
    public:
        QByteArray image;

        void load(QDomDocument& xml) {
            QDomElement chr = xml.firstChildElement("CHARACTER");
            QDomElement img = chr.firstChildElement("IMAGE");
            QString imageText = img.text();
            if (imageText.isEmpty()) return;
            image = QByteArray::fromBase64(imageText.toUtf8());
        }
    };

    BasicConfiguration basicConfiguration;
    CharacterInfo      characterInfo;
    Characteristics    characteristics;
    Skills             skills;
    Perks              perks;
    Talents            talents;
    MartialArts        martialArts;
    Powers             powers;
    Disadvantages      disadvantages;
    Equipment          equipment;
    Image              image;

    int getPrimary(const QString& stat);
    int getSecondary(const QString& stat);
    int getPrimaryResistant(const QString& stat);
    int getSecondaryResistant(const QString& stat);
};

#endif // HSCCHARACTER_H
