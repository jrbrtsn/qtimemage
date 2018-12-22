#ifndef CONFIG_ITEM_H
#define CONFIG_ITEM_H

#include <QtCore>

class configItem
{
  public:
    virtual int parse_line(const QString &line)= 0;
    virtual QString write_line() const = 0;
    virtual bool has_info() const = 0;
    const QString& key() const {return _key;}
  protected:
    configItem(const char *key);
  private:
    QString _key;
};

class configSize : public configItem, public QSize
{
  public:
    configSize(const char *key, int width, int height);
    int parse_line(const QString &line);
    QString write_line() const;
    bool has_info() const;
    QSize& operator=(const QSize &src) {return *(static_cast<QSize*>(this)) = src;}
};

class configRect : public configItem, public QRect
{
  public:
    configRect(const char *key, int x, int y, int width, int height);
    int parse_line(const QString &line);
    QString write_line() const;
    bool has_info() const;
    QRect& operator=(const QRect &src) {return *(static_cast<QRect*>(this)) = src;}
};

class configString : public configItem, public QString
{
  public:
    configString(const char *key);
    int parse_line(const QString &line);
    QString write_line() const;
    bool has_info() const;
    QString& operator=(const QString &src) {return *(static_cast<QString*>(this)) = src;}
};

class configInt : public configItem
{
  public:
    configInt(const char *key, int val);
    int parse_line(const QString &line);
    QString write_line() const;
    bool has_info() const;
    int& operator=(const int &src) {return _val = src;}
    int val() const {return _val;}
    void set_val(int val) {_val= val;}
  private:
    int _val;
};


class configIntVec : public configItem, public QVector<int>
{
  public:
    configIntVec(const char *key);
    int parse_line(const QString &line);
    QString write_line() const;
    bool has_info() const;
    QVector<int>& operator=(const QVector<int> &src) {return *(static_cast<QVector<int>*>(this)) = src;}
};

#endif
