class IProperty 
{
    //--- successful part ---
    //assign enum to metadata
    Q_ENUMS(ConnectionState)
    Q_PROPERTY( ConnectionState state READ readState WRITE writeState NOTIFY notifyState RESET resetState);
public:
    enum ConnectionState {
        Disconnected = 0,
        Connecting,
        Connected,
        Engaged
    };
    ConnectionState readState() const;
    void writeState(ConnectionState state);
    void notifyState();

    //assign flags to metadata
    Q_FLAGS(MyFlags)
    enum enumFlag {
        enumFlag0 = 0,
        enumFlag1,
        enumFlag2,
        enumFlag3
    };
    //define a enum as a flag
    Q_DECLARE_FLAGS(MyFlags, enumFlag)
    Q_PROPERTY( MyFlags Flag READ readFlag NOTIFY notifyFlag);
    MyFlags readFlag() const;
    void notifyFlag();
    void resetState();

    //different function names and missing functions
    Q_ENUMS(Property2Type)
    Q_PROPERTY( Property2Type property2 READ readProperty2 WRITE writeProperty2 NOTIFY notifyProperty2 RESET resetProperty2);
    enum Property2Type{
        prop20 = 0,
        prop21,
        prop22
    };
    Property2Type readProperty2();
    void writeProperty2(Property2Type val);
    void notifyProperty2();
    void resetProperty2();

    //--- failing part
    //wrong enum values
    //different function names and missing functions
    Q_ENUMS(Property3Type)
    Q_PROPERTY( Property3Type property3 READ readProperty3 WRITE writeProperty3 NOTIFY notifyProperty3 RESET resetProperty3);
    enum Property3Type{
        prop30 = 1,
        prop31,
        prop32
    };
    Property3Type readProperty3();
    void writeProperty3(Property3Type val);
    void notifyProperty3();
    void resetProperty3();

    //functions missing
    Q_ENUMS(Property4Type)
    Q_PROPERTY( Property4Type property4 READ readProperty4 WRITE writeProperty4 NOTIFY notifyProperty4 RESET resetProperty4);
    enum Property4Type{
        prop40 = 0,
        prop41,
        prop42
    };
    Property4Type readProperty4();
    void writeProperty4(Property4Type val);
    void notifyProperty4();
    void resetProperty4();

    //wrong enum name
    Q_ENUMS(Property5Type)
    Q_PROPERTY( Property5Type property5 READ readProperty5 WRITE writeProperty5 NOTIFY notifyProperty5 RESET resetProperty5);
    enum Property5Type{
        prop50 = 0,
        prop51,
        prop52
    };
    Property5Type readProperty5();
    void writeProperty5(Property5Type val);
    void notifyProperty5();
    void resetProperty5();

    //using property type without Q_ENUMS
    Q_ENUMS(Property6Type)
    Q_PROPERTY( Property6Type property6 READ readProperty6 WRITE writeProperty6 NOTIFY notifyProperty6 RESET resetProperty6);
    enum Property6Type{
        prop60 = 0,
        prop61,
        prop62
    };
    Property6Type readProperty6();
    void writeProperty6(Property6Type val);
    void notifyProperty6();
    void resetProperty6();

    //using wrong flag name
    Q_FLAGS(MyFlags1)
    enum enumFlag1 {
        enumFlag10 = 0,
        enumFlag11,
        enumFlag12,
        enumFlag13
    };
    //define a enum as a flag
    Q_DECLARE_FLAGS(MyFlags1, enumFlag1)
    Q_PROPERTY( MyFlags1 Flag1 READ readFlag1 NOTIFY notifyFlag1);
    MyFlags1 readFlag1() const;
    void notifyFlag1();

    //using wrong flag value
    Q_FLAGS(MyFlags2)
    enum enumFlag2 {
        enumFlag20 = 1,
        enumFlag21,
        enumFlag22,
        enumFlag23
    };
    //define a enum as a flag
    Q_DECLARE_FLAGS(MyFlags2, enumFlag2)
    Q_PROPERTY( MyFlags2 Flag2 READ readFlag2 NOTIFY notifyFlag2);
    MyFlags2 readFlag2() const;
    void notifyFlag2();
};