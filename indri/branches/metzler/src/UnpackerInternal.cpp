//
// Warning: This file is automatically generated.
//
#include "indri/Unpacker.hpp"
#include "indri/QuerySpec.hpp"
#include "indri/XMLNode.hpp"
#include "lemur/Exception.hpp"

indri::lang::Node* indri::lang::Unpacker::_unpack( XMLNode* child ) {
  Node* result = 0;
  std::string type = child->getAttribute( "type" );
  std::string name = child->getAttribute( "name" );

  if( type == "IndexTerm" ) {
    result = new IndexTerm(*this);
  } else if( type == "Field" ) {
    result = new Field(*this);
  } else if( type == "ExtentInside" ) {
    result = new ExtentInside(*this);
  } else if( type == "ExtentOr" ) {
    result = new ExtentOr(*this);
  } else if( type == "ExtentAnd" ) {
    result = new ExtentAnd(*this);
  } else if( type == "BAndNode" ) {
    result = new BAndNode(*this);
  } else if( type == "UWNode" ) {
    result = new UWNode(*this);
  } else if( type == "ODNode" ) {
    result = new ODNode(*this);
  } else if( type == "FilReqNode" ) {
    result = new FilReqNode(*this);
  } else if( type == "FilRejNode" ) {
    result = new FilRejNode(*this);
  } else if( type == "FieldLessNode" ) {
    result = new FieldLessNode(*this);
  } else if( type == "FieldGreaterNode" ) {
    result = new FieldGreaterNode(*this);
  } else if( type == "FieldBetweenNode" ) {
    result = new FieldBetweenNode(*this);
  } else if( type == "FieldEqualsNode" ) {
    result = new FieldEqualsNode(*this);
  } else if( type == "RawScorerNode" ) {
    result = new RawScorerNode(*this);
  } else if( type == "TermFrequencyScorerNode" ) {
    result = new TermFrequencyScorerNode(*this);
  } else if( type == "CachedFrequencyScorerNode" ) {
    result = new CachedFrequencyScorerNode(*this);
  } else if( type == "PriorNode" ) {
    result = new PriorNode(*this);
  } else if( type == "IdentSimNode" ) {
    result = new IdentSimNode(*this);
  } else if( type == "OrNode" ) {
    result = new OrNode(*this);
  } else if( type == "NotNode" ) {
    result = new NotNode(*this);
  } else if( type == "MaxNode" ) {
    result = new MaxNode(*this);
  } else if( type == "CombineNode" ) {
    result = new CombineNode(*this);
  } else if( type == "WAndNode" ) {
    result = new WAndNode(*this);
  } else if( type == "WSumNode" ) {
    result = new WSumNode(*this);
  } else if( type == "WeightNode" ) {
    result = new WeightNode(*this);
  } else if( type == "ExtentRestriction" ) {
    result = new ExtentRestriction(*this);
  } else if( type == "FilterNode" ) {
    result = new FilterNode(*this);
  } else if( type == "ContextCounterNode" ) {
    result = new ContextCounterNode(*this);
  } else if( type == "ScoreAccumulatorNode" ) {
    result = new ScoreAccumulatorNode(*this);
  } else if( type == "AnnotatorNode" ) {
    result = new AnnotatorNode(*this);
  } else {
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, "Unknown node type '" + type + "' found during unpacking." );
  }

  result->setNodeName( name );
  return result;
}

