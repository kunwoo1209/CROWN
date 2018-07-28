/*
 * Written by Hyunwoo Kim on 2017.07.15
 * clang/llvm version 3.4
 * binary generated from this source file is used by gen_new_cov command.
 * input: .cil.c file
 * output:
 *   bid-lnum-exp: tuples of branch id, line number, expression.
 *   functions_in_file: tuples of file name, function name and bids in it.
 
 * bid-lnum-exp format is as follows.
 *
 * <bid 1> <line 1>
 * <expression 1>
 * <bid 2> <line 2>
 * <expression 2>
 * ...
 *
 * <line n> means the line in original code of if statment
 * coressponding to <bid n>.
 *
 * e.g. For original ifstatement "if ( a && b )" in line 8,
 *      bid-lnum-exp will contain
 *      2 8
 *      a
 *      3 8
 *      b && a
 *      4 8
 *      !(b) && a
 *      5 8
 *      !(a)
 *
 * functions_in_file format is as follows:
 *
 * <file name> <name of func1 in the file>
 * <bid1 in func1> <bid2 in func1> ...
 * <file name> <name of func2 in the file>
 * <bid1 in func2> <bid2 in func2> ...
 * <file name> <name of func3 in the file>
 * <bid1 in func3> <bid2 in func3> ...
 * ...
 *
 * e.g. If target.cil.c has func and main functions,
 *      functions_in_file will contain
 *      target.cil.c func
 *      3 4
 *      target.cil.c main
 *      5 6
 */

#include <cstdio>
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <fstream>
#include <utility>

#include "clang/AST/ParentMap.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/AST/ASTContext.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Frontend/Utils.h"

using namespace clang;
using namespace std;

/*
* g_parentmap is used to get a parent node.
* ParentMap can be composed taking a node s of stmt type as argument.
* ParentMap saves information of parent node for the node s
* and its descendent nodes, while searching subtree rooted at node s.
* i.e. if ParentMap is made for the node s,
* it's possible to get parent node infromation for s and its desendent nodes.
* So, ParentMap needs to be made whenever the node of stmt type,
*  not descendent of any other node of stmt type, is visited.
*
* g_flag decides whether visiting stmt node is descendent of other stmt node.
* g_flag = false if visiting stmt node is descenent of any other stmt node.
* g_flag = true otherwise.
*
* first_visit_func is initialized as true in main.
* It is set to false if a funtion is visited.
*/

ParentMap *g_parentmap;
bool g_flag, first_visit_func;

//fout_1 indicates funtions_in_file.
ofstream fout_1("functions_in_file", ios::app);

//fout_2 indicates bid-lnum-exp.
ofstream fout_2("bid-lnum-exp", ios::app);

class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor>
{
private:
  SourceManager& SMgr;
public:
  MyASTVisitor(SourceManager &_SMgr)
    :SMgr(_SMgr) {}

  bool VisitStmt(Stmt *s) {
    /*
    * if g_flag = false, ParentMap is made and g_flag is set to true
    * to prevent from making ParentMap again for the descendent nodes.
    */
    if(!g_flag){
      g_flag=true;
      //g_parentmap saves newly made ParentMap
      if(g_parentmap != NULL)
        delete g_parentmap;
      g_parentmap = new ParentMap(s);
    }

    //type of visiting node is CallExpr.
    if( isa<CallExpr> (s)){
     /*
      * in VisitStmt, tuples of branch id, line number, expression are printed
      * to bid-lnum-exp.
      */

      string callee_name;
      callee_name = ( (NamedDecl *)( ((CallExpr *)s )-> \
            getCalleeDecl()) )->getNameAsString();

      //name of callee function is __CrownBranch i.e. branch
      if(callee_name == "__CrownBranch"){

        //parsing and printing branch id from __CrownBranch callee.
        Expr *arg_bid=((CallExpr *)s )->getArg(1);
        CharSourceRange arg_bidRange = CharSourceRange::getTokenRange( \
          arg_bid->getLocStart(),arg_bid->getLocEnd());
        string str_bid= Lexer::getSourceText( \
          arg_bidRange, SMgr, LangOptions(), 0);
        fout_2<<str_bid<<" ";
        fout_1<<str_bid<<" ";

        /* str_thenelse saves then("1")/else("0") argument
           from __CrownBranch callee. */
        string str_thenelse;
        
        /*
         * Since compound condition in original code becomes decomposed
         * in .cil.c code, all decomposed conditions should be searched and
         * printed by connecting via &&.
         * Now, we call the Ifstmt node, the closest ancestor node for
         * visiting node(__CresBranch), as strting Ifstmt node.
         * We consider all conditions in Ifstmt nodes from starting Ifstmt up to
         * the one with same line number are in same compound condition.
         *
         * trav_stmt is updated from child to parent node iteratively
         * for searching all decomposed condition.
         */
        Stmt *trav_stmt = s;

        /* 
         * orig_loc_stt_ifstmt saves original line of the starting ifstmt.
         * original line means corresponding line in original code.
         * orig_loc_stt_ifstmt is used to distinguish newly found 
         * ifstmt is in same compound condition with stt_ifstmt
         * by comparing line number.
         */
         unsigned int orig_loc_stt_ifstmt;

        /*
         * is_stt_ifstmt is set to true initially,
         * and set to false after finding other ifstmt among ancestor nodes
         */
        bool is_stt_ifstmt = true;

        /* Searching for IfStmt included in same compound condition with
         * the starting ifstmt, iteratively*/
        do{
          while (true){
            /* get parent node of current trav_stmt */
            trav_stmt = g_parentmap->getParent(trav_stmt);
      
            /* no more decomposed condition exists */
            if(!trav_stmt){
              if(is_stt_ifstmt)
                return false;

              fout_2<<endl;
              return true;
            }

            /* finding ifStmt node */
            if (isa<IfStmt> (trav_stmt))
              break;

            /*
               if or else {
                 __CrownBranch(crownid, bid, then/else)
               }
               __CrownBranch is found.
             */
            else if(isa<CompoundStmt> (trav_stmt) &&
              trav_stmt->child_begin() != trav_stmt->child_end() ) {
                Stmt * child_compound = *(trav_stmt->child_begin());

                if(isa<CallExpr> (child_compound)){
                  callee_name = ( (NamedDecl *)( ((CallExpr *) \
                    child_compound )->getCalleeDecl()) )->getNameAsString();

                  /*
                   * parsing then(1)/else(1) argument from __CrownBranch
                   * to decide whether to negate expression or not.
                   * str_thenelse == "1" if it is then branch.
                   * str_thenelse == "0" if it is else branch.
                   */
                  if(callee_name == "__CrownBranch") {
                    Expr *arg_thenelse=((CallExpr *)child_compound)->getArg(2);
                    CharSourceRange argRange =
                      CharSourceRange::getTokenRange( \
                        arg_thenelse->getLocStart(),arg_thenelse->getLocEnd());
                    str_thenelse= Lexer::getSourceText( \
                      argRange, SMgr, LangOptions(), 0);
                  }
                }
            }
          }

          /* orig_loc_found_ifstmt saves the original line number of
             found ifStmt node */
          unsigned int orig_loc_found_ifstmt = SMgr.getPresumedLoc(\
                  SMgr.getSpellingLoc(trav_stmt->getLocStart())).getLine();

          /* If found Ifstmt is the stt_ifstmt.
             loc_stt_ifstmt is set and printed */
          if(is_stt_ifstmt) {
              orig_loc_stt_ifstmt = orig_loc_found_ifstmt;
              fout_2<<orig_loc_stt_ifstmt<<endl;
          }

          /* Found Ifstmt node isn't stt_ifstmt and
           *  has different line number to stt_ifstmt.
           * i.e. found ifstmt is not in same compound condition
                  with stt_ifstmt, thus finishing search */
          else if(orig_loc_stt_ifstmt != orig_loc_found_ifstmt){
            fout_2<<endl;
            return true;
          }

          /* parse expression from IfStmt and print it */
          Expr *expr = ((IfStmt *)trav_stmt)->getCond();
          CharSourceRange exprRange =
              CharSourceRange::getTokenRange( \
                      expr->getLocStart(), expr->getLocEnd());
          string str_expr= Lexer::getSourceText( \
                  exprRange, SMgr, LangOptions(), 0);

          if(!is_stt_ifstmt)
            fout_2<<"&& ";
          if(str_thenelse=="0")
            fout_2<<"!("<<str_expr<<") ";
          else
            fout_2<<str_expr<<" ";

            is_stt_ifstmt = false;
        }while(1);
      }
    }
    return true;
  }
  /* end of VisitStmt */
  
  bool VisitFunctionDecl(FunctionDecl *f) {
    /*
    * g_flag is set to false.
    * The stmt node to be visited next is body of function,
    * not descendant of any other stmt node.
    * So, ParentMap should be made for the next stmt node.
    */
    g_flag=false;
    if(f->hasBody()) {
      /* __globinit_... function is not added into functions_in_file
         since it is inserted function by cilly. */
      if(f->getNameInfo().getAsString().find("__globinit_") == string::npos) {
        if(!first_visit_func)
          fout_1<<endl;
        first_visit_func = false;
        // print the pair of filename and function name into functions_in_file.
        fout_1<<SMgr.getFilename(SMgr.getImmediateSpellingLoc( \
        f->getLocStart())).str()<<" "<<f->getNameInfo().getAsString()<<endl;
      }
    }

    return true;
  }
};

class MyASTConsumer : public ASTConsumer
{
public:
  MyASTConsumer(SourceManager &SourceMgr)
    : Visitor(SourceMgr) //initialize MyASTVisitor
  {}

  virtual bool HandleTopLevelDecl(DeclGroupRef DR) {
    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b) {
      // Travel each function declaration using MyASTVisitor
      Visitor.TraverseDecl(*b);
    }
    return true;
  }

private:
  MyASTVisitor Visitor;
};


int main(int argc, char *argv[])
{
  if (argc != 2) {
    llvm::errs() << "Usage: parsing_cil <filename>.cil.c\n";
    llvm::errs() << "extension .cil.c means output generated by cilly.\n";
    return -1;
  }

  //initialize first_visit_func to true.
  first_visit_func = true;

  // CompilerInstance will hold the instance of the Clang compiler for us,
  // managing the various objects needed to run the compiler.
  CompilerInstance TheCompInst;
  
  // Diagnostics manage problems and issues in compile 
  TheCompInst.createDiagnostics(NULL, false);

  // Set target platform options 
  // Initialize target info with the default triple for our platform.
  TargetOptions *TO = new TargetOptions();
  TO->Triple = llvm::sys::getDefaultTargetTriple();
  TargetInfo *TI = TargetInfo::CreateTargetInfo( \
      TheCompInst.getDiagnostics(), TO);
  TheCompInst.setTarget(TI);

  // FileManager supports for file system lookup, file system caching,
  // and directory search management.
  TheCompInst.createFileManager();
  FileManager &FileMgr = TheCompInst.getFileManager();
  
  // SourceManager handles loading and caching of source files into memory.
  TheCompInst.createSourceManager(FileMgr);
  SourceManager &SourceMgr = TheCompInst.getSourceManager();
  
  // Prreprocessor runs within a single source file
  TheCompInst.createPreprocessor();
  
  // ASTContext holds long-lived AST nodes (such as types and decls) .
  TheCompInst.createASTContext();

  // Enable HeaderSearch option
  llvm::IntrusiveRefCntPtr<clang::HeaderSearchOptions> \
      hso( new HeaderSearchOptions());
  HeaderSearch headerSearch(hso,
               TheCompInst.getSourceManager(),
               TheCompInst.getDiagnostics(),
               TheCompInst.getLangOpts(),
               TI);

  // <Warning!!> -- Platform Specific Code lives here
  // This depends on A) that you're running linux and
  // B) that you have the same GCC LIBs installed that I do. 
  /*
  $ gcc -xc -E -v -
  ..
  /usr/local/include
  /usr/lib/gcc/x86_64-linux-gnu/4.4.5/include
  /usr/lib/gcc/x86_64-linux-gnu/4.4.5/include-fixed
  /usr/include
  End of search list.
  */
  const char *include_paths[] = {"/usr/local/include",
        "/usr/lib/gcc/x86_64-linux-gnu/4.4.5/include",
        "/usr/lib/gcc/x86_64-linux-gnu/4.4.5/include-fixed",
        "/usr/include"};

  for (int i=0; i<4; i++) 
    hso->AddPath(include_paths[i], 
          clang::frontend::Angled, 
          false, 
          false);
  // </Warning!!> -- End of Platform Specific Code

  InitializePreprocessor(TheCompInst.getPreprocessor(), 
         TheCompInst.getPreprocessorOpts(),
         *hso,
         TheCompInst.getFrontendOpts());


  // A Rewriter helps us manage the code rewriting task.
  Rewriter TheRewriter;
  TheRewriter.setSourceMgr(SourceMgr, TheCompInst.getLangOpts());

  // Set the main file handled by the source manager to the input file.
  const FileEntry *FileIn = FileMgr.getFile(argv[1]);
  SourceMgr.createMainFileID(FileIn);
  
  // Inform Diagnostics that processing of a source file is beginning. 
  TheCompInst.getDiagnosticClient().BeginSourceFile( \
      TheCompInst.getLangOpts(),&TheCompInst.getPreprocessor());
  
  // Create an AST consumer instance which is going to get called by ParseAST.
  MyASTConsumer TheConsumer(SourceMgr);

  // Parse the file to AST, registering our consumer as the AST consumer.
  ParseAST(TheCompInst.getPreprocessor(), &TheConsumer, \
      TheCompInst.getASTContext());
  fout_1.close();
  fout_2.close();

  return 0;
}
