#include "hsccharacter.h"

hscCharacter::hscCharacter() {

}

void hscCharacter::load(QDomDocument& xml) {
    basicConfiguration.load(xml);
         characterInfo.load(xml);
       characteristics.load(xml);
           skills.list.load(xml, "SKILLS");
            perks.list.load(xml, "PERKS");
          talents.list.load(xml, "TALENTS");
      martialArts.list.load(xml, "MARTIALARTS", "MANEUVER");
                powers.load(xml);
    disadvantages.list.load(xml, "DISADVANTAGES", "DISAD");
             equipment.load(xml);
                 image.load(xml);
}

void hscCharacter::BasicConfiguration::load(QDomDocument& xml) {
    QDomElement chr = xml.firstChildElement("CHARACTER");
    QDomElement conf = chr.firstChildElement("BASIC_CONFIGURATION");

    basePoints  = conf.attribute("BASE_POINTS");
    experience  = conf.attribute("EXPERIENCE");
    disadPoints = conf.attribute("DISAD_POINTS");
}

void hscCharacter::CharacterInfo::load(QDomDocument& xml) {
    QDomElement chr = xml.firstChildElement("CHARACTER");
    QDomElement conf = chr.firstChildElement("CHARACTER_INFO");

    background  = conf.firstChildElement("BACKGROUND").text();
    personality = conf.firstChildElement("PERSONALITY").text();
    quote       = conf.firstChildElement("QUOTE").text();
    tactics     = conf.firstChildElement("TACTICS").text();
    campaignUse = conf.firstChildElement("CAMPAIGN_USE").text();
    appearance  = conf.firstChildElement("APPEARANCE").text();

    alternateIdentities = conf.attribute("ALTERNATE_IDENTITIES");
    hairColor           = conf.attribute("HAIR_COLOR");
    playerName          = conf.attribute("PLAYER_NAME");
    height              = conf.attribute("HEIGHT");
    genre               = conf.attribute("GENRE");
    characterName       = conf.attribute("CHARACTER_NAME");
    eyeColor            = conf.attribute("EYE_COLOR");
    gm                  = conf.attribute("GM");
    campaignName        = conf.attribute("CAMPAIGN_NAME");
    weight              = conf.attribute("WEIGHT");
}

void hscCharacter::Base::load(QDomElement& e) {
    auto nodes = e.attributes();
    int count = nodes.count();
    for (int i = 0; i < count; ++i) {
        QDomAttr node = nodes.item(i).toAttr();
        if (node.isNull()) continue;
        attributes[node.name()] = node.value();
    }
    attributes["NOTES"] = e.firstChildElement("NOTES").text();
}

int hscCharacter::getPrimary(const QString& stat) {
    QMap<QString, int> base {
        { "STR", 10 }, { "DEX", 10 }, { "CON", 10 }, { "INT", 10 }, { "EGO", 10 }, { "PRE", 10 }, { "OCV", 3 }, { "DCV", 3 }, { "OMCV", 3 }, { "DMCV", 3 },
        { "SPD", 2 }, { "PD", 2 }, { "ED", 2 }, { "REC", 4 }, { "END", 20 }, { "BODY", 10 }, { "STUN", 20 }, { "RUNNING", 12 }, { "SWIMMING", 4 }, { "LEAPING", 4 }
    };
    int level = base[stat] + characteristics.characteristics[stat].getLevels() + powers.getPrimaries(stat);
    if (stat == "PD" || stat == "ED") {
        level += talents.getPrimaries("COMBAT_LUCK");
        level += powers.getPrimaries("FORCEFIELD", stat);
    }
    return level;
}

int hscCharacter::getSecondary(const QString& stat) {
    int level = powers.getSecondaries(stat);
    if (stat == "PD" || stat == "ED") {
        level += talents.getSecondaries("COMBAT_LUCK");
        level += powers.getSecondaries("FORCEFIELD", stat);
    }
    return level;
}

int hscCharacter::getPrimaryResistant(const QString& stat) {
    int level = 0;
    level += talents.getPrimaries("COMBAT_LUCK");
    level += powers.getPrimaries("FORCEFIELD", stat);
    return level;
}

int hscCharacter::getSecondaryResistant(const QString& stat) {
    int level = 0;
    level += talents.getSecondaries("COMBAT_LUCK");
    level += powers.getSecondaries("FORCEFIELD", stat);
    return level;
}

int hscCharacter::Talents::getPrimaries(const QString& statName) {
    int primary = 0;
    for (const auto& talent: list) {
        if (talent.attributes["XMLID"] == statName) {
            if (talent.attributes["AFFECTS_TOTAL"] == "Yes" &&
                talent.attributes["AFFECTS_PRIMARY"] == "Yes") primary += talent.attributes["LEVELS"].toInt() * 3;
        }
    }
    return primary;
}

int hscCharacter::Talents::getSecondaries(const QString& statName) {
    int secondary = 0;
    for (const auto& talent: list) {
        if (talent.attributes["XMLID"] == statName) {
            if (talent.attributes["AFFECTS_TOTAL"] == "Yes" &&
                talent.attributes["AFFECTS_PRIMARY"] == "No") secondary += talent.attributes["LEVELS"].toInt() * 3;
        }
    }
    return secondary;
}

int hscCharacter::Powers::getPrimaries(const QString& statName) {
    int primary = 0;
    for (auto& power: items) {
        Stat* stat = std::get_if<Stat>(&power);
        if (stat == nullptr) continue;
        if (stat->attributes["XMLID"] == statName) {
            if (stat->attributes["AFFECTS_TOTAL"] == "Yes" &&
                stat->attributes["AFFECTS_PRIMARY"] == "Yes") primary += stat->attributes["LEVELS"].toInt();
        }
    }
    return primary;
}

int hscCharacter::Powers::getSecondaries(const QString& statName) {
    int secondary = 0;
    for (auto& power: items) {
        Stat* stat = std::get_if<Stat>(&power);
        if (stat == nullptr) continue;
        if (stat->attributes["XMLID"] == statName) {
            if (stat->attributes["AFFECTS_TOTAL"] == "Yes" &&
                stat->attributes["AFFECTS_PRIMARY"] == "No") secondary += stat->attributes["LEVELS"].toInt();
        }
    }
    return secondary;
}

int hscCharacter::Powers::getPrimaries(const QString& powerName, const QString& where) {
    int primary = 0;
    for (auto& power: items) {
        Power* pwr = std::get_if<Power>(&power);
        if (pwr == nullptr) continue;
        if (pwr->attributes["XMLID"] == powerName) {
            if (pwr->attributes["AFFECTS_TOTAL"] == "Yes" &&
                pwr->attributes["AFFECTS_PRIMARY"] == "Yes") primary += pwr->attributes[where + "LEVELS"].toInt();
        }
    }
    return primary;
}

int hscCharacter::Powers::getSecondaries(const QString& powerName, const QString& where) {
    int secondary = 0;
    for (auto& power: items) {
        Power* pwr = std::get_if<Power>(&power);
        if (pwr == nullptr) continue;
        if (pwr->attributes["XMLID"] == powerName) {
            if (pwr->attributes["AFFECTS_TOTAL"] == "Yes" &&
                pwr->attributes["AFFECTS_PRIMARY"] == "No") secondary += pwr->attributes[where + "LEVELS"].toInt();
        }
    }
    return secondary;
}
