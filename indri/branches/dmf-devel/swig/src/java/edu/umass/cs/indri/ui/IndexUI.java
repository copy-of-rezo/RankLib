package edu.umass.cs.indri.ui;
import javax.swing.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.util.*;
import edu.umass.cs.indri.*;

/**
   User interface for building Indri indexes.
   @author David Fisher
   @version 1.0
*/
public class IndexUI extends JPanel implements ActionListener, 
					       ItemListener, CaretListener
{
    /** Status line messages visible for all tabs. */
    private JLabel status;
    /** Messages pane for prog output */
    private JTextArea messages;
    /** Top level container */
    private JTabbedPane tabbedPane;
    /** Single choice */
    private JCheckBox doRecurse, doStem;
    /** Multiple choices */
    private JComboBox stemmers,	memoryLim, docFormat;
    /** Action Buttons */
    private JButton browse, cfbrowse, stopBrowse, cfRemove, go, stop;
    /** input/browseable filename */
    private JTextField iname, stopwordlist;
    /** field names */
    private JTextField colFields, indFields;
	
    /** filename filter */
    private JTextField filterString;
    /** browse multiple filenames */
    private JList collectionFiles;
    /** hold the file names */
    private DefaultListModel cfModel;
    /** File chooser starting in current directory */
    private final JFileChooser fc = 
	new JFileChooser(System.getProperties().getProperty("user.dir"));
    /** data directory paths */    
    private String stopwords;
    /** Help file for the application */
    private final static String helpFile = "properties/IndriIndex.html";
    /** The little icon */
    private final static String iconFile = "properties/lemur_icon.GIF";
    /** The big logo */
    private final static String logoFile = null;
    /** Indri FileClassEnvironments */
    private final static String [] formats = {"trecweb", "trectext", "html",
					      "doc", "ppt", "pdf", "txt"};
    /** Memory limit choices */
    private final static String [] lims = {"  64 MB", "  96 MB", " 128 MB", 
					   " 256 MB", " 512 MB", "1024 MB"};
    /** Stemmer types */
    private final static String[] sTypes = {"krovetz", "porter"};
	
    /** are we appending? */
    boolean appendIndex = false;
	
    /** MenuBar */
    private JMenuBar menuBar;
    /** Menus */
    private JMenu fileMenu, helpMenu;
    /** Menu Items */
    private JMenuItem fOpen, fSave, fPrefs, fQuit, hHelp, hAbout;
    /** The Indri Icon. */
    private ImageIcon indriIcon;
    /** About the indexer. */
    private final static String aboutText = "Indri Indexer UI 1.0\n" +
	"Copyright (c) 2004 University of Massachusetts";
    /** Frame for help window */
    private JFrame helpFrame;
	
    /** Get the ball rolling. */
    public IndexUI () {	
	super(new BorderLayout());
	initGUI();
    }
    /**
     * Initialize the GUI elements, including preloading the
     * help frame.
     */
    private void initGUI() {	
	// starting with a JPanel using BorderLayout
	// indexing tab to use GridBagLayout
	//	indriIcon = createImageIcon(iconFile);
	// reuse for each labeled component.
	JLabel label;
	// set up icon images
	indriIcon = null;
	ImageIcon icon = null;  // no icon on tabs
	// initialize help
	makeHelp();
	tabbedPane = new JTabbedPane();
	// initialize the indexing tab
		
	JComponent panel = new JPanel();
	GridBagLayout layout = new GridBagLayout();
	GridBagConstraints constraints = new GridBagConstraints();
	//	constraints.fill = GridBagConstraints.HORIZONTAL;
	constraints.anchor = GridBagConstraints.LINE_END;
	panel.setLayout(layout);	
		
	// browse button for index name
	browse = new JButton("Browse...");
	browse.addActionListener(this);
	browse.setToolTipText("Browse to a directory and enter a basename " +
			      "for the index");
	// index name
	iname = new JTextField("", 25);
	iname.setToolTipText("Enter a basename for the index or browse to " +
			     "a directory");
	iname.addCaretListener(this);
	label = new JLabel("Index Name: ", JLabel.TRAILING);
	label.setLabelFor(iname);
	// set GridBagConstraints.
	//	constraints.weightx = 0.5;
	//	constraints.ipadx = 10; // side to side separation
	//	constraints.ipady = 20; // over under separation
	// should use insets
	constraints.insets = new Insets(10,10,0,0);
	constraints.gridx = 0;
	constraints.gridy = 0;
	panel.add(label, constraints);
	constraints.gridx = 1;
	constraints.anchor = GridBagConstraints.LINE_START;
	panel.add(iname, constraints);
	constraints.gridx = 2;
	panel.add(browse, constraints);
	constraints.anchor = GridBagConstraints.LINE_END;
	// data files list
	cfModel = new DefaultListModel();
	collectionFiles = new JList(cfModel);
	collectionFiles.setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
	String fill = "123456789012345678901234567890123456789";
	collectionFiles.setPrototypeCellValue(fill);
	collectionFiles.setVisibleRowCount(5);
	collectionFiles.setToolTipText("Browse to a directory and select " + 
				       "input files or directories.");
	JScrollPane listScrollPane = new JScrollPane(collectionFiles);
	// browse button for data files
	cfbrowse = new JButton("Browse...");
	cfbrowse.addActionListener(this);
	cfbrowse.setToolTipText("Browse to a directory and select input " + 
				"files or directories.");
	// remove buttone for data files
	cfRemove = new JButton("Remove");
	cfRemove.addActionListener(this);
	cfRemove.setToolTipText("Remove selected files from the list.");
	// second row
	constraints.gridy = 1;
	label = new JLabel("Data File(s): ", JLabel.TRAILING);
	constraints.gridx = 0;
	panel.add(label, constraints);
	constraints.gridx = 1;
	panel.add(listScrollPane, constraints);
	// check box for recurse into subdirectories
	doRecurse = new JCheckBox("Recurse into subdirectories");
	doRecurse.setToolTipText("When checked and a directory is in the " + 
				 "data files list, recursively add all " +
				 "data files in that directory and all of" + 
				 " its subbdirectories into the data " +
				 "files list.");
	JPanel cfButtons = new JPanel(new BorderLayout());
	cfButtons.add(cfbrowse, BorderLayout.NORTH);
	// button layout needs some spacing, bleah. Gridbag it too?
	cfButtons.add(cfRemove, BorderLayout.SOUTH);
	constraints.gridx = 2;
	constraints.anchor = GridBagConstraints.LINE_START;
	panel.add(cfButtons, constraints);
	constraints.anchor = GridBagConstraints.LINE_END;
	// third row
	constraints.gridy = 2;
	label = new JLabel("Filename filter: ", JLabel.TRAILING);
	constraints.gridx = 0;
	panel.add(label, constraints);
	filterString = new JTextField("", 25);
	label.setLabelFor(filterString);
	filterString.setToolTipText("Specify a filename filter, eg *.sgml");
	constraints.gridx = 1;
	constraints.anchor = GridBagConstraints.LINE_START;
	panel.add(filterString, constraints);
	constraints.anchor = GridBagConstraints.LINE_END;
	constraints.gridx = 2;
	panel.add(doRecurse, constraints);
		
	// fourth row
	constraints.gridy = 3;
		
	colFields = new JTextField("docno", 25);
	colFields.setToolTipText("Comma delimited list of field names, " +
				 "without spaces");
		
	indFields = new JTextField("title", 25);
	indFields.setToolTipText("Comma delimited list of field names, " +
				 "without spaces");
	label = new JLabel("Collection Fields: ", JLabel.TRAILING);
	label.setLabelFor(colFields);
	constraints.gridx = 0;
	panel.add(label, constraints);
	constraints.gridx = 1;
	constraints.anchor = GridBagConstraints.LINE_START;
	panel.add(colFields, constraints);
	constraints.anchor = GridBagConstraints.LINE_END;
		
	// fifth row
	constraints.gridy = 4;
		
	label = new JLabel("Indexed Fields: ", JLabel.TRAILING);
	label.setLabelFor(colFields);
	constraints.gridx = 0;
	panel.add(label, constraints);
	constraints.gridx = 1;
	constraints.anchor = GridBagConstraints.LINE_START;
	panel.add(indFields, constraints);
	constraints.anchor = GridBagConstraints.LINE_END;
		
	// sixth row
	constraints.gridy = 5;
	docFormat = new JComboBox(formats);
	docFormat.setToolTipText("Select format of input files");	
	docFormat.addActionListener(this);
	label = new JLabel("Document format: ", JLabel.TRAILING);
	constraints.gridx = 0;
	panel.add(label, constraints);
	constraints.gridx = 1;
	constraints.anchor = GridBagConstraints.LINE_START;
	panel.add(docFormat, constraints);
	constraints.anchor = GridBagConstraints.LINE_END;
	// seventh row
	constraints.gridy = 6;
		
	stopwordlist = new JTextField(stopwords, 25);
	stopwordlist.setToolTipText("Enter a stopword list or browse to " +
				    "select. Clear this field if you do " +
				    "not want to stop this index.");
	stopBrowse = new JButton("Browse...");
	stopBrowse.addActionListener(this);
	stopBrowse.setToolTipText("Browse to a directory and select " +
				  "a stoplist.");
	label = new JLabel("Stopword list: ", JLabel.TRAILING);
	constraints.gridx = 0;
	panel.add(label, constraints);
		
	constraints.gridx = 1;
	constraints.anchor = GridBagConstraints.LINE_START;
	panel.add(stopwordlist, constraints);
	constraints.gridx = 2;
	panel.add(stopBrowse, constraints);
	constraints.anchor = GridBagConstraints.LINE_END;
		
	// eighth row
	constraints.gridy = 7;
		
	doStem = new JCheckBox("Stem collection", true);
	doStem.addItemListener(this);
	doStem.setToolTipText("Select to enable stemming (conflation " +
			      "of morphological variants) for this index");
		
	stemmers = new JComboBox(sTypes);
	stemmers.setToolTipText("Select stemming algorithm.");
	stemmers.addActionListener(this);
		
	constraints.gridx = 0;
	panel.add(doStem, constraints);
		
	//	label = new JLabel("Stemmer: ", JLabel.TRAILING);
	//	constraints.gridx = 1;
	//	panel.add(label, constraints);
		
	//	constraints.gridx = 2;
	constraints.gridx = 1;
	constraints.anchor = GridBagConstraints.LINE_START;
	panel.add(stemmers, constraints);
	constraints.anchor = GridBagConstraints.LINE_END;
		
	memoryLim = new JComboBox(lims);
	memoryLim.setToolTipText("How much memory to use while indexing. " +
				 "A rule of thumb is no more than 3/4 of " +
				 "your physical memory");
	memoryLim.setSelectedIndex(2); // 128 MB
	label = new JLabel("Memory limit: ", JLabel.TRAILING);
	constraints.gridx = 2;
	constraints.anchor = GridBagConstraints.LINE_START;
	panel.add(label, constraints);
	constraints.gridx = 3;
	constraints.anchor = GridBagConstraints.LINE_END;
	panel.add(memoryLim, constraints);
		
	//put it all on a single tab, reorg and align nicely.
	//Adjust constraints for the content pane.
		
	tabbedPane.addTab("Index", icon, panel, "Index Options");
		
	JPanel panel4 = makePanel();
	messages = new JTextArea(10,40);
	messages.setEditable(false);
		
	JScrollPane messageScrollPane = 
	    new JScrollPane(messages, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
			    JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
	panel4.add(messageScrollPane);
	tabbedPane.addTab("Status", icon, panel4, "Status Messages");
		
	JPanel buttons = new JPanel();
	go = new JButton("Build Index");
	go.setEnabled(false);
	go.addActionListener(this);	
	stop = new JButton("Quit");
	stop.addActionListener(this);
	buttons.add(go);
	buttons.add(stop);
	status = new JLabel("Ready...", indriIcon, JLabel.LEFT);
	add(tabbedPane, BorderLayout.NORTH);
	add(buttons, BorderLayout.CENTER);
	add(status, BorderLayout.SOUTH);
    }
    // gui helper functions.
    /** Create the applications menu bar.
	@return the JMenuBar.
    */    
    private JMenuBar makeMenuBar() {
	// menu
	menuBar = new JMenuBar();
	fileMenu = new JMenu("File");
	helpMenu = new JMenu("Help");
	menuBar.add(fileMenu);
	menuBar.add(Box.createHorizontalGlue());
	menuBar.add(helpMenu);
	fQuit = makeMenuItem("Quit");
	fileMenu.add(fQuit);
	hHelp = makeMenuItem("Help");
	helpMenu.add(hHelp);
	hAbout = makeMenuItem("About");
	helpMenu.add(hAbout);
	return menuBar;
    }
    /** Creates a JMenuItem with this as its ActionListener.
	@param label the label for the item.
	@return the created menu item.
    */    
    private JMenuItem makeMenuItem(String label) {
	JMenuItem item 	= new JMenuItem(label);
	item.addActionListener(this);
	return item;
    }
	
    /** Create a JPanel with BorderLayout.
	@return the Jpanel.
    */    
    private JPanel makePanel() {
	JPanel panel = new JPanel(new BorderLayout());
	return panel;
    }
	
    /** Returns an ImageIcon, or null if the path was invalid. 
	@param path the image file to load.
	@return an ImageIcon, or null if the path was invalid. 
    */
    protected static ImageIcon createImageIcon(String path) {
	java.net.URL imgURL = IndexUI.class.getResource(path);
	if (imgURL != null) {
	    return new ImageIcon(imgURL);
	} else {
	    return null;
	}
    }
	
    //Listeners
    /** Omnibus for responding to user actions. */
    public void actionPerformed(ActionEvent e) {
	Object source = e.getSource();
	if (source == browse) 	{
	    fc.setFileSelectionMode(JFileChooser.FILES_AND_DIRECTORIES);
	    int returnVal = fc.showOpenDialog(this);
	    if (returnVal == JFileChooser.APPROVE_OPTION) {
		File file = fc.getSelectedFile();
		iname.setText(file.getAbsolutePath());
	    }
	    fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
	} else if (source == cfbrowse) 	{
	    // pick file or directory
	    fc.setFileSelectionMode(JFileChooser.FILES_AND_DIRECTORIES);
	    // pick multiple at the same time
	    fc.setMultiSelectionEnabled(true);
	    // final because the inner class uses it.
	    final String regexp = filterString.getText();
	    if (regexp.length() > 0) {
		final String filtString = encodeRegexp(regexp);
		javax.swing.filechooser.FileFilter filt = 
		    new javax.swing.filechooser.FileFilter() {
			public boolean accept(File f) {
			    if (f.isDirectory()) {
				return true;
			    }
			    String name = f.getName();
			    return name.matches(filtString);
			}
			public String getDescription() {
			    return regexp + " files";
			}
		    };
		fc.addChoosableFileFilter(filt);
	    }
			
	    int returnVal = fc.showOpenDialog(this);
	    if (returnVal == JFileChooser.APPROVE_OPTION) {
		File [] files = fc.getSelectedFiles();
		for (int i = 0; i < files.length; i++)
		    cfModel.addElement(files[i].getAbsolutePath());
	    }
	    fc.setMultiSelectionEnabled(false);
	    fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
	    // remove the filter.
	    fc.resetChoosableFileFilters();
	} else if (source == stopBrowse) {
	    int returnVal = fc.showOpenDialog(this);
	    if (returnVal == JFileChooser.APPROVE_OPTION) {
		File file = fc.getSelectedFile();
		stopwordlist.setText(file.getAbsolutePath());
		stopwords = stopwordlist.getText();
	    }
	} else if (source == stemmers) 	{
	} else if (source == docFormat) {
	} else if (source == go) 	{
	    // show parameters and build index.
	    // sanity check first.
	    if (! safeToBuildIndex()) {
		// need more completed messages.
		status.setText("Unable to build " + iname.getText());
		return;
	    }
	    // flip to status tab -- change to 1 if only 2 tabs
	    tabbedPane.setSelectedIndex(1);
	    // start a new thread to run in so messages will be updated.
	    Runnable r = new Runnable() {
		    public void run() {
			buildIndex();
			ensureMessagesVisible();
		    }
		};
	    Thread t = new Thread(r);
	    t.start();
	} else if (source == stop) 	{
	    System.exit(0);
	} else if (source == fQuit) 	{
	    System.exit(0);
	} else if (source == cfRemove) 	{
	    Object [] selected = collectionFiles.getSelectedValues();
	    for (int i = 0; i < selected.length; i++) {
		cfModel.removeElement(selected[i]);
	    }
	} else if (source == hHelp) 	{
	    // pop up a help dialog
	    helpFrame.setVisible(true);
	} else if (source == hAbout) 	{
	    JOptionPane.showMessageDialog(this, aboutText, "About", 
					  JOptionPane.INFORMATION_MESSAGE);
	    //  createImageIcon(logoFile));
	}
	// at least one datafile and a name entered.
	boolean enabled = (cfModel.getSize() > 0 && 
			   iname.getText().length() > 0);
	go.setEnabled(enabled);
    }
	
    /** Listens to the check boxes. */
    public void itemStateChanged(ItemEvent e) {
	Object source = e.getItemSelectable();
	boolean change = (e.getStateChange() == ItemEvent.SELECTED);
	if (source == doStem) stemmers.setEnabled(change);
    }
	
    /** Listens to the index name text field. This enables updating the
	state of the BuildIndex button when the user types in the name
	of the index directly.
    */
    public void caretUpdate(CaretEvent event) {
	boolean enabled = (cfModel.getSize() > 0 && 
			   iname.getText().length() > 0);
	go.setEnabled(enabled);
    }
	
    /** Create the frame that shows the help file and render the html.
     */
    private void makeHelp() {
	java.net.URL helpURL = IndexUI.class.getResource(helpFile);
	JTextPane help = new JTextPane();
		
	//Create and set up the window.
	helpFrame = new JFrame("Indri Index Builder Help");
	help.setPreferredSize(new Dimension(650, 400));
	help.setEditable(false);
	help.addHyperlinkListener(new DocLinkListener());
	JScrollPane scroller =
	    new JScrollPane(help, 
			    JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
			    JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
	try {
	    help.setPage(helpURL);
	} catch (IOException ex) {
	    help.setText("Help file unavailable.");
	}

	helpFrame.getContentPane().add(scroller, BorderLayout.CENTER);
	helpFrame.setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
	//	helpFrame.setIconImage(createImageIcon(iconFile).getImage());
	helpFrame.pack();
    }
	
	
    /**
     * Create the GUI and show it.  For thread safety,
     * this method should be invoked from the
     * event-dispatching thread.
     */
    private static void createAndShowGUI() {
	//Make sure we have nice window decorations.
	JFrame.setDefaultLookAndFeelDecorated(true);
	// For system look and feel
	/*
	  try {
	  UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
	  } catch (Exception e) { 
	  }
	*/
	//Create and set up the window.
	JFrame frame = new JFrame("Indri Index Builder");
	frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	frame.setIconImage(createImageIcon(iconFile).getImage());
	//Create and set up the content pane.
	IndexUI newContentPane = new IndexUI();
	newContentPane.setOpaque(true); //content panes must be opaque
	frame.getContentPane().add(newContentPane, BorderLayout.CENTER);
	frame.setJMenuBar(newContentPane.makeMenuBar());
	//Display the window.
	frame.pack();
	frame.setVisible(true);
    }
	
    // Utilities    
    /** Rewrite a shell filename pattern to a regular expression. <br>
     *  * -&gt; .*<br>
     ? -&gt; .?<br>
     Add ^ to beginning<br>
     Add $ to end<br>
     . -&gt; \.<br>
     @param regexp the filename pattern, eg "*.dat"
     @return a regular expression suitable for use with String.matches(), 
     eg "^.*\.dat$"
    */
    private String encodeRegexp(String regexp) {
	// rewrite shell fname pattern to regexp.
	// * -> .*
	// ? -> .?
	// Add ^,$
	// . -> \.
	String retval = "^" + regexp + "$";
	retval = retval.replaceAll("\\.", "\\.");
	retval = retval.replaceAll("\\*", ".*");
	retval = retval.replaceAll("\\?", ".?");
	return retval;
    }
	
    /** Tests for likely failure scenarios. If the path to the index 
	doesn't exist, fail. If the named index already exists, offer
	a choice do either overwrite, append, or abandon. If overwrite is 
	selected, removes all files in the target directory.
	This is potentially dangerous.
	@return true if it is safe to build the index otherwise false.
    */
    private boolean safeToBuildIndex() {
	appendIndex = false;
	// if iname is not an absolute path, rewrite it to be so.
	File idx = new File(iname.getText());
	String idxPath = idx.getAbsolutePath();
	idx = new File(idxPath);
	// if parent directory does not exist (typein error)
	// fail
	File dir = idx.getParentFile();
	if (!dir.exists()) {
	    messages.append("Unable to build " + idxPath +
			    "Directory " + dir.getAbsolutePath() +
			    " does not exist.\n");
	    return false;
	}
	// if idx exists, either fail or blow it away.
	// have to look inside for the manifest, etc.
	File manifest = new File(idxPath, "manifest");
	if (manifest.exists()) {
	    // need an append option (and attribute to hold it for open
	    // versus create call.
	    int val =
		JOptionPane.showConfirmDialog(this,
					      manifest.getAbsolutePath() + " exists. Choose YES to overwrite, NO to append to this index, CANCEL to do nothing",
					      "Overwrite or Append existing index",
					      JOptionPane.YES_NO_CANCEL_OPTION,
					      JOptionPane.WARNING_MESSAGE);
	    if (val == JOptionPane.CANCEL_OPTION) {
		// don't overwrite or append
		messages.append("Not building index " + idxPath + "\n");
		return false;
	    } else if (val == JOptionPane.NO_OPTION) {
		// don't overwrite. append
		messages.append("Appending new files to index " + idxPath + 
				"\n");
		appendIndex = true;
		return true;
	    } else if (val == JOptionPane.YES_OPTION) {
		// overwrite -- delete all index files.
		messages.append("Overwriting index " + idxPath + "\n");
		deleteDirectory(idx);
		return true;
	    } else { // any other option
		messages.append("Not building index " + idxPath + "\n");
		return false;
	    }
	}
	return true;
    }
    /** remove a directory and all of its files and subdirectories */
    private void deleteDirectory(File dir) {
	File [] files = dir.listFiles();
	for (int i = 0; i < files.length; i++) {
	    File f = files[i];
	    if (f.isDirectory())
		deleteDirectory(f);
	    messages.append("Deleting: " + f.getAbsolutePath() + "\n");
	    f.delete();
	}
    }
	
    /** Keeps the message pane showing the last line appended. */
    private void ensureMessagesVisible() {
	int len = messages.getText().length();
	try {
	    messages.scrollRectToVisible(messages.modelToView(len));
	} catch (javax.swing.text.BadLocationException ex) {
	    // don't care.
	}
    }
	
    /** Ask the IndexEnvironment to add the files.*/
    private void buildIndex() {
	Cursor wait = Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR);
	Cursor def = Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR);
	setCursor(wait);
	messages.setCursor(wait);
	// if iname is not an absolute path, rewrite it to be so.
	File idx = new File(iname.getText());
	iname.setText(idx.getAbsolutePath());
	messages.append("Building " + iname.getText() + "...\n"); 
	status.setText("Building " + iname.getText() + "..."); 
	Thread bl = blinker(status.getText(), 
			    "Finished building " + iname.getText());
	// construct IndexEnvironment
	// set parameters
	// go.
	IndexEnvironment env = new IndexEnvironment();
	IndexStatus stat = new UIIndexStatus();
	// memory
	env.setMemory(encodeMem());
		
	String [] fields = indFields.getText().split(",");;
	String [] metafields = colFields.getText().split(",");;
	String [] stopwords = new String[0];
	env.setIndexedFields(fields);
	env.setMetadataIndexedFields(metafields);	
	String stops = stopwordlist.getText();
	if (! stops.equals("")) {
	    // load the stopwords into an array.
	    Vector tmp = new Vector();
	    try {
		BufferedReader buf = new BufferedReader(new FileReader(stops));
		String line;
		while((line = buf.readLine()) != null) {
		    tmp.add(line.trim());
		}
		buf.close();
	    } catch (IOException ex) {
		// unlikely.
		showException(ex);
	    }
	    stopwords = (String[]) tmp.toArray(stopwords);
	    env.setStopwords(stopwords);
	}
		
	if (doStem.isSelected()) {
	    String stemmer = (String)stemmers.getSelectedItem();
	    env.setStemmer(stemmer);
	}
		
	String fileClass = (String)docFormat.getSelectedItem();
	String [] datafiles = formatDataFiles();
		
	// create a new empty index (after parameters have been set).
	if (appendIndex)
	    env.open(iname.getText(), stat);
	else 
	    env.create(iname.getText(), stat);
		
	// don't let 'em quit easy while it is running.
	fQuit.setEnabled(false);
	stop.setEnabled(false);
	// do the building.
	for (int i = 0; i < datafiles.length; i++){
	    String fname = datafiles[i];
	    env.addFile(fname, fileClass);
	    ensureMessagesVisible();
	}
	env.close();
	// now they can quit.
	fQuit.setEnabled(true);
	stop.setEnabled(true);
	blinking = false;
	bl.interrupt();
	setCursor(def);
	messages.setCursor(def);
	status.setText("Finished building " + iname.getText());
	messages.append("Finished building " + iname.getText() + "\n\n");
	ensureMessagesVisible();
    }
	
	
    /** Create the datafiles list of strings.
	@return The list of files
    */
    private String[] formatDataFiles() {
	// handle directories, recursion, filename patterns	
	Vector accumulator = new Vector();
	String [] retval = new String[0];
		
	FileFilter filt = null;
	final String regexp = filterString.getText();
	if (regexp.length() > 0) {
	    final String filtString = encodeRegexp(regexp);
	    filt = new FileFilter() {
		    public boolean accept(File thisfile) {
			String name = thisfile.getName();
			return (thisfile.isDirectory() ||
				name.matches(filtString));
		    }
		};
	}
	Enumeration e = cfModel.elements();
	while (e.hasMoreElements()) {
	    String s = (String) e.nextElement();
	    File file = new File(s);
	    formatDataFiles(file, filt, accumulator);		
	}
	retval = (String[]) accumulator.toArray(retval);
	return retval;
    }
	
    /** Accumulate filenames for the input list.
	If the File is a directory, iterate over all of the files
	in that directory that satisfy the filter. If recurse into
	subdirectories is selected and the File is a directory, 
	invoke recursivly on on all directories within the directory.
	@param accum Vector to accumulate file names recusively.
	@param file a File (either a file or directory)
	@param f the filename filter to apply.
    */
	
    private void formatDataFiles(File file, FileFilter f, Vector accum) {
	if (file.isDirectory()) {
	    // handle directory
	    File [] files = file.listFiles(f);
	    for (int i = 0; i < files.length; i++) {
		if (files[i].isDirectory()) {
		    if (doRecurse.isSelected()) {
			formatDataFiles(files[i], f, accum);
		    }
		} else {
		    accum.add(files[i].getAbsolutePath());
		}
	    }
	} else {
	    accum.add(file.getAbsolutePath());
	}
    }
	
    private long encodeMem() {
	String s = ((String)memoryLim.getSelectedItem()).trim();
	int space = s.indexOf(' ');
	s = s.substring(0, space) + "000000";
	long retval = 0;
	try {
	    retval = Long.parseLong(s);
	} catch (Exception e) {
	}
	return retval;
    }
	
    /** Format an exception message in the messages text area.
	@param e the exception
    */
    private void showException(Exception e) {
	messages.append("\nERROR:\n");
	StringWriter msg = new StringWriter();
	PrintWriter w = new PrintWriter(msg);
	e.printStackTrace(w);
	w.close();
	messages.append(msg.toString());
	ensureMessagesVisible();
    }
	
    /** Is the blinker running? */
    private volatile boolean blinking = false;
    /** Make the status line blink while working. */
    private Thread blinker(final String s1, final String s2) {
	Thread blink;
	blink = new Thread(new Runnable() {
		public void run() {
		    String onText = s1;
		    String doneText = s2;
		    String offText = "";
		    int count = 0;
		    try {
			while (blinking) {
			    Thread.sleep(500);
			    if (count%2 == 0) {
				status.setText(offText);
			    } else {
				status.setText(onText);
			    }
			    count++;
			}
		    } catch (InterruptedException ex) {
			status.setText(doneText);
		    }
		}
	    });
	blinking = true;
	blink.start();
	return blink;
    }
    /** Fire it up.*/    
    public static void main(String[] args) {
	//Schedule a job for the event-dispatching thread:
	//creating and showing this application's GUI.
	javax.swing.SwingUtilities.invokeLater(new Runnable() {
		public void run() {
		    createAndShowGUI();
		}
	    });
    }
    class UIIndexStatus extends IndexStatus {
	public void status(int code, String documentFile, String error, 
			   int documentsIndexed, int documentsSeen) {
	    if (code == IndexStatus.FileOpen) {
		messages.append("Documents: " + documentsIndexed + "\n");
		messages.append("Opened " + documentFile + "\n");
	    } else if (code == IndexStatus.FileSkip) {
		messages.append("Skipped " + documentFile + "\n");
	    } else if (code == IndexStatus.FileError) {
		messages.append("Error in " + documentFile + " : " + error + 
				"\n");
	    } else if (code == IndexStatus.DocumentCount) {
		if( (documentsIndexed % 50) == 0)
		    messages.append( "Documents: " + documentsIndexed + "\n" );
	    }
	    int len = messages.getText().length();
	    try {
		messages.scrollRectToVisible(messages.modelToView(len));
	    } catch (javax.swing.text.BadLocationException ex) {
		// don't care.
	    }
	}
    }
}
