target_sources (mlbookproc
    PRIVATE AddBook.h
    PRIVATE ArchEntry.h
    PRIVATE ArchiveFileEntry.h
    PRIVATE ArchiveRemoveEntry.h
    PRIVATE ARCHParser.h
    PRIVATE AuxFunc.h
    PRIVATE BaseKeeper.h
    PRIVATE BookBaseEntry.h
    PRIVATE BookInfoEntry.h
    PRIVATE BookInfo.h
    PRIVATE BookMarks.h
    PRIVATE BookParseEntry.h
    PRIVATE ByteOrder.h
    PRIVATE CreateCollection.h
    PRIVATE DCParser.h
    PRIVATE DJVUParser.h
    PRIVATE ElectroBookInfoEntry.h
    PRIVATE EPUBParser.h
    PRIVATE FB2Parser.h
    PRIVATE FileParseEntry.h
    PRIVATE FormatAnnotation.h
    PRIVATE GenreGroup.h
    PRIVATE Genre.h
    PRIVATE Hasher.h
    PRIVATE LibArchive.h
    PRIVATE MLException.h
    PRIVATE NotesBaseEntry.h
    PRIVATE NotesKeeper.h
    PRIVATE ODTParser.h
    PRIVATE OpenBook.h
    PRIVATE PaperBookInfoEntry.h
    PRIVATE PDFParser.h
    PRIVATE RefreshCollection.h
    PRIVATE RemoveBook.h
    PRIVATE ReplaceTagItem.h
    PRIVATE SelfRemovingPath.h
    PRIVATE XMLParser.h
    PRIVATE XMLTag.h
)

if(USE_OPENMP)
    target_sources(mlbookproc PRIVATE OmpLockGuard.h)
endif()
