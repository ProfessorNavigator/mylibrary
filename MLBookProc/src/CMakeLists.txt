target_sources (mlbookproc
    PRIVATE AddBook.cpp
    PRIVATE ArchEntry.cpp
    PRIVATE ArchiveFileEntry.cpp
    PRIVATE ArchiveRemoveEntry.cpp
    PRIVATE ARCHParser.cpp
    PRIVATE AuxFunc.cpp
    PRIVATE BaseKeeper.cpp
    PRIVATE BookBaseEntry.cpp
    PRIVATE BookInfo.cpp
    PRIVATE BookInfoEntry.cpp
    PRIVATE BookMarks.cpp
    PRIVATE BookParseEntry.cpp
    PRIVATE ByteOrder.cpp
    PRIVATE CreateCollection.cpp
    PRIVATE DCParser.cpp
    PRIVATE DJVUParser.cpp
    PRIVATE ElectroBookInfoEntry.cpp
    PRIVATE EPUBParser.cpp
    PRIVATE FB2Parser.cpp
    PRIVATE FileParseEntry.cpp
    PRIVATE FormatAnnotation.cpp
    PRIVATE Genre.cpp
    PRIVATE GenreGroup.cpp
    PRIVATE Hasher.cpp
    PRIVATE LibArchive.cpp
    PRIVATE MLException.cpp
    PRIVATE NotesBaseEntry.cpp
    PRIVATE NotesKeeper.cpp
    PRIVATE ODTParser.cpp
    PRIVATE OpenBook.cpp
    PRIVATE PaperBookInfoEntry.cpp
    PRIVATE PDFParser.cpp
    PRIVATE RefreshCollection.cpp
    PRIVATE RemoveBook.cpp
    PRIVATE ReplaceTagItem.cpp
    PRIVATE SelfRemovingPath.cpp
    PRIVATE XMLParser.cpp
    PRIVATE XMLTag.cpp
)

if(USE_OPENMP)
    target_sources(mlbookproc PRIVATE OmpLockGuard.cpp)
endif()
