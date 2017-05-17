#-------------------------------------------------
#
# Project created by QtCreator 2016-11-26T14:52:34
#
#-------------------------------------------------

QT       -= core gui

TEMPLATE = lib
CONFIG += staticlib

DESTDIR = $$PWD/../StaticLib

# x86 and x64 specific macros defines
CONFIG += debug_and_release
contains(QT_ARCH, i386) {
    DEFINES += ARCH_X86
    CONFIG(debug, debug|release){
        TARGET = RakNet_x86d
    }else{
        TARGET = RakNet_x86
    }
}
else{
    DEFINES += ARCH_X64
    CONFIG(debug, debug|release){
        TARGET = RakNet_x64d
    }else{
        TARGET = RakNet_x64
    }
}

SOURCES += \
    _FindFirst.cpp \
    Base64Encoder.cpp \
    BitStream.cpp \
    CCRakNetSlidingWindow.cpp \
    CCRakNetUDT.cpp \
    CheckSum.cpp \
    CloudClient.cpp \
    CloudCommon.cpp \
    CloudServer.cpp \
    CommandParserInterface.cpp \
    ConnectionGraph2.cpp \
    ConsoleServer.cpp \
    DataCompressor.cpp \
    DirectoryDeltaTransfer.cpp \
    DR_SHA1.cpp \
    DS_BytePool.cpp \
    DS_ByteQueue.cpp \
    DS_HuffmanEncodingTree.cpp \
    DS_Table.cpp \
    DynDNS.cpp \
    EmailSender.cpp \
    EpochTimeToString.cpp \
    FileList.cpp \
    FileListTransfer.cpp \
    FileOperations.cpp \
    FormatString.cpp \
    FullyConnectedMesh2.cpp \
    Getche.cpp \
    Gets.cpp \
    GetTime.cpp \
    gettimeofday.cpp \
    GridSectorizer.cpp \
    HTTPConnection.cpp \
    HTTPConnection2.cpp \
    IncrementalReadInterface.cpp \
    Itoa.cpp \
    LinuxStrings.cpp \
    LocklessTypes.cpp \
    LogCommandParser.cpp \
    MessageFilter.cpp \
    NatPunchthroughClient.cpp \
    NatPunchthroughServer.cpp \
    NatTypeDetectionClient.cpp \
    NatTypeDetectionCommon.cpp \
    NatTypeDetectionServer.cpp \
    NetworkIDManager.cpp \
    NetworkIDObject.cpp \
    PacketConsoleLogger.cpp \
    PacketFileLogger.cpp \
    PacketizedTCP.cpp \
    PacketLogger.cpp \
    PacketOutputWindowLogger.cpp \
    PluginInterface2.cpp \
    PS4Includes.cpp \
    Rackspace.cpp \
    RakMemoryOverride.cpp \
    RakNetCommandParser.cpp \
    RakNetSocket.cpp \
    RakNetSocket2.cpp \
    RakNetSocket2_360_720.cpp \
    RakNetSocket2_Berkley.cpp \
    RakNetSocket2_Berkley_NativeClient.cpp \
    RakNetSocket2_NativeClient.cpp \
    RakNetSocket2_PS3_PS4.cpp \
    RakNetSocket2_PS4.cpp \
    RakNetSocket2_Vita.cpp \
    RakNetSocket2_Windows_Linux.cpp \
    RakNetSocket2_Windows_Linux_360.cpp \
    RakNetSocket2_WindowsStore8.cpp \
    RakNetStatistics.cpp \
    RakNetTransport2.cpp \
    RakNetTypes.cpp \
    RakPeer.cpp \
    RakSleep.cpp \
    RakString.cpp \
    RakThread.cpp \
    RakWString.cpp \
    Rand.cpp \
    RandSync.cpp \
    ReadyEvent.cpp \
    RelayPlugin.cpp \
    ReliabilityLayer.cpp \
    ReplicaManager3.cpp \
    Router2.cpp \
    RPC4Plugin.cpp \
    SecureHandshake.cpp \
    SendToThread.cpp \
    SignaledEvent.cpp \
    SimpleMutex.cpp \
    SocketLayer.cpp \
    StatisticsHistory.cpp \
    StringCompressor.cpp \
    StringTable.cpp \
    SuperFastHash.cpp \
    TableSerializer.cpp \
    TCPInterface.cpp \
    TeamBalancer.cpp \
    TeamManager.cpp \
    TelnetTransport.cpp \
    ThreadsafePacketLogger.cpp \
    TwoWayAuthentication.cpp \
    UDPForwarder.cpp \
    UDPProxyClient.cpp \
    UDPProxyCoordinator.cpp \
    UDPProxyServer.cpp \
    VariableDeltaSerializer.cpp \
    VariableListDeltaTracker.cpp \
    VariadicSQLParser.cpp \
    VitaIncludes.cpp \
    WSAStartupSingleton.cpp

HEADERS += \
    _FindFirst.h \
    AutopatcherPatchContext.h \
    AutopatcherRepositoryInterface.h \
    Base64Encoder.h \
    BitStream.h \
    CCRakNetSlidingWindow.h \
    CCRakNetUDT.h \
    CheckSum.h \
    CloudClient.h \
    CloudCommon.h \
    CloudServer.h \
    CommandParserInterface.h \
    ConnectionGraph2.h \
    ConsoleServer.h \
    DataCompressor.h \
    DirectoryDeltaTransfer.h \
    DR_SHA1.h \
    DS_BinarySearchTree.h \
    DS_BPlusTree.h \
    DS_BytePool.h \
    DS_ByteQueue.h \
    DS_Hash.h \
    DS_Heap.h \
    DS_HuffmanEncodingTree.h \
    DS_HuffmanEncodingTreeFactory.h \
    DS_HuffmanEncodingTreeNode.h \
    DS_LinkedList.h \
    DS_List.h \
    DS_Map.h \
    DS_MemoryPool.h \
    DS_Multilist.h \
    DS_OrderedChannelHeap.h \
    DS_OrderedList.h \
    DS_Queue.h \
    DS_QueueLinkedList.h \
    DS_RangeList.h \
    DS_Table.h \
    DS_ThreadsafeAllocatingQueue.h \
    DS_Tree.h \
    DS_WeightedGraph.h \
    DynDNS.h \
    EmailSender.h \
    EmptyHeader.h \
    EpochTimeToString.h \
    Export.h \
    FileList.h \
    FileListNodeContext.h \
    FileListTransfer.h \
    FileListTransferCBInterface.h \
    FileOperations.h \
    FormatString.h \
    FullyConnectedMesh2.h \
    Getche.h \
    Gets.h \
    GetTime.h \
    gettimeofday.h \
    GridSectorizer.h \
    HTTPConnection.h \
    HTTPConnection2.h \
    IncrementalReadInterface.h \
    InternalPacket.h \
    Itoa.h \
    Kbhit.h \
    LinuxStrings.h \
    LocklessTypes.h \
    LogCommandParser.h \
    MessageFilter.h \
    MessageIdentifiers.h \
    MTUSize.h \
    NativeFeatureIncludes.h \
    NativeFeatureIncludesOverrides.h \
    NativeTypes.h \
    NatPunchthroughClient.h \
    NatPunchthroughServer.h \
    NatTypeDetectionClient.h \
    NatTypeDetectionCommon.h \
    NatTypeDetectionServer.h \
    NetworkIDManager.h \
    NetworkIDObject.h \
    PacketConsoleLogger.h \
    PacketFileLogger.h \
    PacketizedTCP.h \
    PacketLogger.h \
    PacketOutputWindowLogger.h \
    PacketPool.h \
    PacketPriority.h \
    PluginInterface2.h \
    PS3Includes.h \
    PS4Includes.h \
    Rackspace.h \
    RakAlloca.h \
    RakAssert.h \
    RakMemoryOverride.h \
    RakNetCommandParser.h \
    RakNetDefines.h \
    RakNetDefinesOverrides.h \
    RakNetSmartPtr.h \
    RakNetSocket.h \
    RakNetSocket2.h \
    RakNetStatistics.h \
    RakNetTime.h \
    RakNetTransport2.h \
    RakNetTypes.h \
    RakNetVersion.h \
    RakPeer.h \
    RakPeerInterface.h \
    RakSleep.h \
    RakString.h \
    RakThread.h \
    RakWString.h \
    Rand.h \
    RandSync.h \
    ReadyEvent.h \
    RefCountedObj.h \
    RelayPlugin.h \
    ReliabilityLayer.h \
    ReplicaEnums.h \
    ReplicaManager3.h \
    Router2.h \
    RPC4Plugin.h \
    SecureHandshake.h \
    SendToThread.h \
    SignaledEvent.h \
    SimpleMutex.h \
    SimpleTCPServer.h \
    SingleProducerConsumer.h \
    SocketDefines.h \
    SocketIncludes.h \
    SocketLayer.h \
    StatisticsHistory.h \
    StringCompressor.h \
    StringTable.h \
    SuperFastHash.h \
    TableSerializer.h \
    TCPInterface.h \
    TeamBalancer.h \
    TeamManager.h \
    TelnetTransport.h \
    ThreadPool.h \
    ThreadsafePacketLogger.h \
    TransportInterface.h \
    TwoWayAuthentication.h \
    UDPForwarder.h \
    UDPProxyClient.h \
    UDPProxyCommon.h \
    UDPProxyCoordinator.h \
    UDPProxyServer.h \
    VariableDeltaSerializer.h \
    VariableListDeltaTracker.h \
    VariadicSQLParser.h \
    VitaIncludes.h \
    WindowsIncludes.h \
    WSAStartupSingleton.h \
    XBox360Includes.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
