target_sources (mylibrary
    PRIVATE AddBookGui.h
    PRIVATE AddBookModelItem.h
    PRIVATE BookInfoGui.h
    PRIVATE BookMarksGui.h
    PRIVATE BookMarksModelItem.h
    PRIVATE BookMarksShow.h
    PRIVATE CollectionCrProcessGui.h
    PRIVATE CopyBookGui.h
    PRIVATE CoverPixBuf.h
    PRIVATE CreateCollectionGui.h
    PRIVATE EditBookGenreModelItem.h
    PRIVATE EditBookGui.h
    PRIVATE EmptyCollectionGui.h
    PRIVATE ExportCollectionGui.h
    PRIVATE FullSizeCover.h
    PRIVATE ImportCollectionGui.h
    PRIVATE LeftGrid.h
    PRIVATE MainWindow.h
    PRIVATE MyLibraryApplication.h
    PRIVATE NotesGui.h
    PRIVATE RefreshCollectionGui.h
    PRIVATE RemoveBookGui.h
    PRIVATE RemoveCollectionGui.h
    PRIVATE RightGrid.h
    PRIVATE SaveCover.h
    PRIVATE SearchProcessGui.h
    PRIVATE SearchResultModelItemAuth.h
    PRIVATE SearchResultModelItemFL.h
    PRIVATE SearchResultModelItem.h
    PRIVATE SearchResultShow.h
    PRIVATE SettingsWindow.h
    PRIVATE TransferBookGui.h
)

if(USE_PLUGINS)
target_sources(mylibrary
    PRIVATE PluginsKeeper.h
)
endif()
