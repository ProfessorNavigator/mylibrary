target_sources(mylibrary
    PRIVATE AddBookGui.cpp
    PRIVATE AddBookModelItem.cpp
    PRIVATE BookInfoGui.cpp
    PRIVATE BookMarksGui.cpp
    PRIVATE BookMarksModelItem.cpp
    PRIVATE BookMarksShow.cpp
    PRIVATE BooksWindow.cpp
    PRIVATE CollectionCrProcessGui.cpp
    PRIVATE CopyBookGui.cpp
    PRIVATE CoverPixBuf.cpp
    PRIVATE CreateCollectionGui.cpp
    PRIVATE EditBookGenreModelItem.cpp
    PRIVATE EditBookGui.cpp
    PRIVATE EmptyCollectionGui.cpp
    PRIVATE ExportCollectionGui.cpp
    PRIVATE FullSizeCover.cpp
    PRIVATE ImportCollectionGui.cpp
    PRIVATE LeftGrid.cpp
    PRIVATE main.cpp
    PRIVATE MainWindow.cpp   
    PRIVATE MagickModelItem.cpp
    PRIVATE MyLibraryApplication.cpp
    PRIVATE NotesGui.cpp
    PRIVATE RefreshCollectionGui.cpp
    PRIVATE RemoveBookGui.cpp
    PRIVATE RemoveCollectionGui.cpp
    PRIVATE RightGrid.cpp
    PRIVATE SaveCover.cpp
    PRIVATE SearchProcessGui.cpp
    PRIVATE SearchResultModelItem.cpp
    PRIVATE SearchResultModelItemAuth.cpp
    PRIVATE SearchResultModelItemFL.cpp
    PRIVATE SearchResultShow.cpp
    PRIVATE SettingsWindow.cpp
    PRIVATE TransferBookGui.cpp
)

if(USE_PLUGINS)
target_sources(mylibrary
    PRIVATE PluginsKeeper.cpp
)
endif()
