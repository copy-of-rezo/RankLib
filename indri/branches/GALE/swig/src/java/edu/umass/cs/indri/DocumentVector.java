package edu.umass.cs.indri;

/**
 * DocumentVector
 *
 * 10 August 2004 -- tds
 */

public class DocumentVector {
  static public class Field {
    public int begin;
    public int end;
    public long number;
    public String name;
  };

  public String[] stems;
  public int[] positions;
  public Field[] fields;
}
